#include "sdl3/impl/renderer_factory/renderer_factory_sdl3_gpu.h"

#include "core/util/uploader_type.h"
#include "core/util/log.h"

#include "graphics/components/size.h"
#include "graphics/inf/render_command.h"
#include "graphics/inf/render_view.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/inf/snippet_factory.h"
#include "renderer/inf/snippet.h"
#include "renderer/util/render_util.h"

#include "app/base/application_context.h"

#include "sdl3/base/context_sdl3_gpu.h"
#include "sdl3/impl/buffer/buffer_sdl3_gpu.h"
#include "sdl3/impl/pipeline_factory/pipeline_factory_sdl3_gpu.h"
#include "sdl3/impl/texture/texture_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

namespace {

class SnippetSDL3_GPU final : public Snippet {
public:
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineDescriptor& pipelineDescriptor) override {
        const String sLocation = "location";
        const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;

        RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);

        const PipelineLayout& pipelineLayout = pipelineDescriptor.layout();
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
        {
            RenderUtil::setLayoutDescriptor(vertex->_declaration_images, "binding", static_cast<uint32_t>(pipelineLayout.ubos().size() + pipelineLayout.ssbos().size() + pipelineLayout.samplers().size()));
            vertex->_predefined_macros.emplace_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT))
        {
            fragment->linkNextStage("FragColor");
            RenderUtil::setLayoutDescriptor(fragment->_declaration_images, "binding", static_cast<uint32_t>(fragment->_declaration_samplers.vars().size()));

            uint32_t binding = 0;
            const Vector<String> samplerNames = fragment->_declaration_samplers.vars().keys();
            fragment->_declaration_samplers.clear();
            for(const String& k : samplerNames)
            {
                constexpr uint32_t samplerSet = 2;
                fragment->_declaration_samplers.declare("sampler", "", k + "_S", Strings::sprintf("set = %d, binding = %d", samplerSet, binding));
                fragment->_declaration_samplers.declare("texture2D", "", k + "_T", Strings::sprintf("set = %d, binding = %d", samplerSet, binding++));
                fragment->_predefined_macros.emplace_back(Strings::sprintf("#define %s sampler2D(%s_T, %s_S)", k.c_str(), k.c_str(), k.c_str()));
            }
        }

        if(const ShaderPreprocessor* compute = context.computingStage().get())
        {
            const uint32_t bindingOffset = static_cast<uint32_t>(pipelineLayout.ubos().size() + pipelineLayout.ssbos().size());
            RenderUtil::setLayoutDescriptor(compute->_declaration_images, "binding", bindingOffset);
        }

        const ShaderPreprocessor* prestage = nullptr;
        for(auto iter = context.renderStages().begin(); iter != context.renderStages().end(); ++iter)
        {
            if(prestage)
            {
                RenderUtil::setLayoutDescriptor(prestage->_declaration_outs, iter->second->_declaration_ins, sLocation, 0);
                RenderUtil::setLayoutDescriptor(iter->second->_declaration_outs, sLocation, 0);
            }
            prestage = iter->second.get();
        }

        {
            Map<uint32_t, uint32_t> spaces;
            for(const sp<PipelineLayout::UBO>& i : pipelineLayout.ubos())
            {
                uint32_t& binding = spaces[i->_stages.bits()];
                i->_binding = binding;
                ++ binding;
            }
        }

        for(const auto& [k, v] : context.renderStages())
        {
            ShaderPreprocessor& preprocessor = v;
            preprocessor._version = 450;
            preprocessor.declareUBOStruct(pipelineLayout, k == enums::SHADER_STAGE_BIT_VERTEX ? 1 : 3);
            preprocessor._predefined_macros.push_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor._predefined_macros.push_back("#extension GL_ARB_shading_language_420pack : enable");
        }
    }
};

class SnippetFactorySDL3 final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override
    {
        return sp<Snippet>::make<SnippetSDL3_GPU>();
    }
};

void setVersion(const enums::RendererVersion version, RenderEngineContext& vkContext)
{
    LOGD("Choose Renderer Version = %d", version);
    vkContext.setSnippetFactory(sp<SnippetFactory>::make<SnippetFactorySDL3>());
    vkContext.setVersion(version);
}

Optional<SDL_GPUDepthStencilTargetInfo> toDepthStencilTargetInfo(const RenderTarget::Configure& configure)
{
    if(!configure._depth_stencil_attachment)
        return {};

    const SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {
        nullptr,
        1.0f,
        !configure._depth_stencil_op || configure._depth_stencil_op == RenderTarget::ATTACHMENT_OP_BIT_DONT_CARE ? SDL_GPU_LOADOP_DONT_CARE
                                                : configure._depth_stencil_op.has(RenderTarget::ATTACHMENT_OP_BIT_CLEAR) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD,
        configure._depth_stencil_op.has(RenderTarget::ATTACHMENT_OP_BIT_STORE) ? SDL_GPU_STOREOP_STORE : SDL_GPU_STOREOP_DONT_CARE,
        configure._clear_bits.has(RenderTarget::CLEAR_BIT_STENCIL) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_DONT_CARE,
        SDL_GPU_STOREOP_DONT_CARE,
    };
    return {depthStencilTargetInfo};
}

class RenderCommandOffscreenPredraw final : public RenderCommand {
public:
    RenderCommandOffscreenPredraw(RenderTarget::Configure configure)
        : _configure(std::move(configure)), _depth_stencil_target(toDepthStencilTargetInfo(_configure))
    {
        const SDL_GPULoadOp loadOp = configure._color_attachment_op == RenderTarget::ATTACHMENT_OP_BIT_DONT_CARE ? SDL_GPU_LOADOP_DONT_CARE
                                                : configure._color_attachment_op.has(RenderTarget::ATTACHMENT_OP_BIT_CLEAR) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
        const SDL_GPUStoreOp storeOp = configure._color_attachment_op.has(RenderTarget::ATTACHMENT_OP_BIT_STORE) ? SDL_GPU_STOREOP_STORE : SDL_GPU_STOREOP_DONT_CARE;
        for(const sp<Texture>& _ : _configure._color_attachments)
            _render_targets.push_back({
                nullptr,
                0,
                0,
                {0, 0, 0, 0},
                loadOp,
                storeOp
            });
    }

    void draw(GraphicsContext& graphicsContext) override
    {
        for(size_t i = 0; i < _render_targets.size(); ++i)
            _render_targets.at(i).texture = reinterpret_cast<SDL_GPUTexture*>(_configure._color_attachments.at(i)->id());
        if(_configure._depth_stencil_attachment)
            _depth_stencil_target->texture = reinterpret_cast<SDL_GPUTexture*>(_configure._depth_stencil_attachment->id());

        GraphicsContextSDL3_GPU& gc = ensureGraphicsContext(graphicsContext);
        gc.pushRenderTargets(&_configure, _render_targets, _depth_stencil_target);
    }

private:
    RenderTarget::Configure _configure;
    Vector<SDL_GPUColorTargetInfo> _render_targets;
    Optional<SDL_GPUDepthStencilTargetInfo> _depth_stencil_target;
};

class RenderCommandOffscreenPostdraw final : public RenderCommand {
public:
    void draw(GraphicsContext& graphicsContext) override
    {
        GraphicsContextSDL3_GPU& gc = ensureGraphicsContext(graphicsContext);
        gc.popRenderTargets();
    }
};

class OffscreenRendererSDL3_GPU final : public Renderer {
public:
    OffscreenRendererSDL3_GPU(sp<Renderer> renderer, RenderTarget::Configure configure)
        : _delegate(std::move(renderer)), _pre_draw(sp<RenderCommand>::make<RenderCommandOffscreenPredraw>(std::move(configure))), _post_draw(sp<RenderCommand>::make<RenderCommandOffscreenPostdraw>())
    {
    }

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override
    {
        renderRequest.addRenderCommand(_pre_draw);
        _delegate->render(renderRequest, position, drawDecorator);
        renderRequest.addRenderCommand(_post_draw);
    }

private:
    sp<Renderer> _delegate;
    sp<RenderCommand> _pre_draw;
    sp<RenderCommand> _post_draw;
};

class RenderViewSDL3_GPU final : public RenderView {
public:
    RenderViewSDL3_GPU(sp<RenderEngineContext> renderContext, sp<RenderController> renderController)
        : _graphics_context(new GraphicsContext(std::move(renderContext), std::move(renderController)))
    {
    }

    void onSurfaceCreated() override
    {
    }

    void onSurfaceChanged(uint32_t width, uint32_t height) override
    {
        _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));
    }

    void onRenderFrame(V4 backgroundColor, RenderCommand& renderCommand) override
    {
        const ContextSDL3_GPU& context = ensureContext(_graphics_context);
        GraphicsContextSDL3_GPU& graphicsContext = ensureGraphicsContext(_graphics_context);

        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context._gpu_gevice);
        if(!cmdbuf)
        {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return;
        }

        graphicsContext._command_buffer = cmdbuf;
        SDL_GPUTexture* swapchainTexture = nullptr;
        if(SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context._main_window, &swapchainTexture, nullptr, nullptr))
        {
            DASSERT(swapchainTexture);
            const Vector<SDL_GPUColorTargetInfo> colorTargets = {{
                swapchainTexture,
                0,
                0,
                { backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), backgroundColor.w() },
                SDL_GPU_LOADOP_CLEAR,
                SDL_GPU_STOREOP_STORE
            }};
            constexpr Optional<SDL_GPUDepthStencilTargetInfo> depthStencilTarget = {};
            graphicsContext.pushRenderTargets(nullptr, colorTargets, depthStencilTarget);
            _graphics_context->onDrawFrame();
            renderCommand.draw(_graphics_context);
            graphicsContext.popRenderTargets();
        }
        else
            SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());

        SDL_SubmitGPUCommandBuffer(cmdbuf);
    }

private:
    op<GraphicsContext> _graphics_context;
};

}

RendererFactorySDL3_GPU::RendererFactorySDL3_GPU()
    : RendererFactory({{enums::RENDERING_BACKEND_BIT_DIRECT_X, enums::RENDERING_BACKEND_BIT_METAL}, enums::COORDINATE_SYSTEM_RHS, false, 16}), _gpu_device(nullptr)
{
}

void RendererFactorySDL3_GPU::onSurfaceCreated(RenderEngine& renderEngine)
{
    _gpu_device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
#if ARK_FLAG_DEBUG
            true,
#else
            false,
#endif
            nullptr);
    CHECK(_gpu_device, "GPUCreateDevice failed");

    ContextSDL3_GPU& sdl3GPUContext = renderEngine.context()->traits().ensure<ContextSDL3_GPU>();
    ASSERT(sdl3GPUContext._main_window);
    sdl3GPUContext._gpu_gevice = _gpu_device;

    const bool success = SDL_ClaimWindowForGPUDevice(_gpu_device, sdl3GPUContext._main_window);
    CHECK(success, "SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
}

sp<RenderEngineContext> RendererFactorySDL3_GPU::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    setVersion(renderer._version == enums::RENDERER_VERSION_AUTO ? enums::RENDERER_VERSION_VULKAN_13 : renderer._version, renderContext);
    return renderContext;
}

sp<Buffer::Delegate> RendererFactorySDL3_GPU::createBuffer(const Buffer::Usage usage)
{
    if(usage.has(Buffer::USAGE_BIT_STORAGE))
        return sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE);
    if(usage.has(Buffer::USAGE_BIT_VERTEX))
        return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_VERTEX) : sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_VERTEX);
    if(usage.has(Buffer::USAGE_BIT_INDEX))
        return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDEX) : sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDEX);
    if(usage.has(Buffer::USAGE_BIT_DRAW_INDIRECT))
        return sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDIRECT);
    FATAL("Unknow buffer type: %d", usage.bits());
    return nullptr;
}

sp<Camera::Delegate> RendererFactorySDL3_GPU::createCamera(const enums::CoordinateSystem rcs)
{
    return rcs == enums::COORDINATE_SYSTEM_LHS ? sp<Camera::Delegate>::make<Camera::DelegateLH_NO>() :  sp<Camera::Delegate>::make<Camera::DelegateRH_NO>();
}

sp<RenderTarget> RendererFactorySDL3_GPU::createRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure)
{
    return sp<RenderTarget>::make(sp<Renderer>::make<OffscreenRendererSDL3_GPU>(std::move(renderer), std::move(configure)), nullptr);
}

sp<PipelineFactory> RendererFactorySDL3_GPU::createPipelineFactory()
{
    return sp<PipelineFactory>::make<PipelineFactorySDL3_GPU>();
}

sp<RenderView> RendererFactorySDL3_GPU::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::make<RenderViewSDL3_GPU>(renderContext, renderController);
}

sp<Texture::Delegate> RendererFactorySDL3_GPU::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D || parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<Texture::Delegate>::make<TextureSDL3_GPU>(static_cast<uint32_t>(size->widthAsFloat()), static_cast<uint32_t>(size->heightAsFloat()), std::move(parameters));
    return nullptr;
}

sp<RendererFactory> RendererFactorySDL3_GPU::BUILDER::build(const Scope& args)
{
    return sp<RendererFactory>::make<RendererFactorySDL3_GPU>();
}

}
