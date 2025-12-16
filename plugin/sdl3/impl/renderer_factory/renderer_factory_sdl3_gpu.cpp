#include "sdl3/impl/renderer_factory/renderer_factory_sdl3_gpu.h"

#include <ranges>

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

#include "sdl3/base/sdl3_context.h"
#include "sdl3/base/context_sdl3_gpu.h"
#include "sdl3/impl/buffer/buffer_sdl3_gpu.h"
#include "sdl3/impl/pipeline_factory/pipeline_factory_sdl3_gpu.h"
#include "sdl3/impl/texture/texture_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

namespace {

class SnippetSDL3_GPU final : public Snippet {
public:
    void postInitialize(PipelineBuildingContext& context) override
    {
        const String sLocation = "location";

        if(!context.renderStages().empty())
        {
            const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;
            RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);
        }

        const PipelineLayout& pipelineLayout = context._pipeline_layout;
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
        {
            CHECK(vertex->_declaration_images.vars().empty(), "Vertex shader must not contain storage images");
            vertex->_predefined_macros.emplace_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT))
        {
            fragment->linkNextStage("FragColor");

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

        const ShaderPreprocessor* prestage = nullptr;
        for(const op<ShaderPreprocessor>& stage : context.renderStages() | std::views::values)
        {
            if(prestage)
            {
                RenderUtil::setLayoutDescriptor(prestage->_declaration_outs, stage->_declaration_ins, sLocation, 0);
                RenderUtil::setLayoutDescriptor(stage->_declaration_outs, sLocation, 0);
            }
            prestage = stage.get();
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

        for(ShaderPreprocessor* preprocessor : context.stages())
        {
            preprocessor->_version = 450;
            preprocessor->declareUBOStruct(pipelineLayout, preprocessor->_shader_stage == enums::SHADER_STAGE_BIT_VERTEX ? 1 : (preprocessor->_shader_stage == enums::SHADER_STAGE_BIT_COMPUTE ? 2 : 3));
            preprocessor->_predefined_macros.push_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor->_predefined_macros.push_back("#extension GL_ARB_shading_language_420pack : enable");
            preprocessor->_predefined_macros.emplace_back("#define ARK_USE_DX");

            // We declare both sampler and texture in _declaration_samplers so the actual number of samplers should be half of the size.
            const int32_t samplerCount = preprocessor->_declaration_samplers.vars().size() / 2;
            if(!preprocessor->_ssbos.empty())
            {
                CHECK(preprocessor->_shader_stage != enums::SHADER_STAGE_BIT_VERTEX, "SSBO should not be declared in vertex shaders");
                if(preprocessor->_shader_stage == enums::SHADER_STAGE_BIT_FRAGMENT)
                {
                    int32_t ssboBindingLocation = samplerCount + preprocessor->_declaration_images.vars().size();
                    for(auto& [k, v] : preprocessor->_ssbos)
                    {
                        CHECK(v._binding._qualifier.contains(enums::SHADER_TYPE_QUALIFIER_READONLY), "Pixel shader buffer \"%s\" should be declared as readonly", k.c_str());
                        RenderUtil::overrideLayoutDescriptor(k, v._declaration, v._binding, ssboBindingLocation++, 2, "Readonly buffer", "pixel shaders");
                    }
                }
                else
                {
                    //TODO: Readwrite buffer should be after storage images offset;
                    int32_t ssboBindingLocation = 0;
                    int32_t readonlySSBOBindingLocation = preprocessor->_declaration_images.vars().size();
                    for(auto& [k, v] : preprocessor->_ssbos)
                    {
                        if(v._binding._qualifier.contains(enums::SHADER_TYPE_QUALIFIER_READONLY))
                            RenderUtil::overrideLayoutDescriptor(k, v._declaration, v._binding, readonlySSBOBindingLocation++, 0, "Readonly buffer", "compute shaders");
                        else
                            RenderUtil::overrideLayoutDescriptor(k, v._declaration, v._binding, ssboBindingLocation++, 1, "Readwrite buffer", "compute shaders");
                    }
                }
            }
            if(preprocessor->_shader_stage == enums::SHADER_STAGE_BIT_FRAGMENT)
            {
                int32_t imageBindingLocation = samplerCount;
                for(const auto& [k, v] : preprocessor->_declaration_images.vars())
                {
                    CHECK(v._binding._qualifier.contains(enums::SHADER_TYPE_QUALIFIER_READONLY), "Pixel shader images \"%s\" should be declared as readonly", k.c_str());
                    RenderUtil::overrideLayoutDescriptor(k, v._source, v._binding, imageBindingLocation++, 2, "Image", "pixel shaders");
                }
            }
            else
            {
                int32_t imageBindingLocation = 0;
                int32_t readonlyImageBindingLocation = samplerCount;
                for(const auto& [k, v] : preprocessor->_declaration_images.vars())
                {
                    if(v._binding._qualifier.contains(enums::SHADER_TYPE_QUALIFIER_READONLY))
                        RenderUtil::overrideLayoutDescriptor(k, v._source, v._binding, readonlyImageBindingLocation++, 0, "Readonly image", "shaders");
                    else
                        RenderUtil::overrideLayoutDescriptor(k, v._source, v._binding, imageBindingLocation++, 1, "Readwrite image", "shaders");
                }
            }
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
        !configure._depth_attachment_op || configure._depth_attachment_op == RenderTarget::ATTACHMENT_OP_BIT_DONT_CARE ? SDL_GPU_LOADOP_DONT_CARE
                                                : configure._depth_attachment_op.contains(RenderTarget::ATTACHMENT_OP_BIT_CLEAR) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD,
        configure._depth_attachment_op.contains(RenderTarget::ATTACHMENT_OP_BIT_STORE) ? SDL_GPU_STOREOP_STORE : SDL_GPU_STOREOP_DONT_CARE,
        SDL_GPU_LOADOP_CLEAR,
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
                                                : configure._color_attachment_op.contains(RenderTarget::ATTACHMENT_OP_BIT_CLEAR) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
        const SDL_GPUStoreOp storeOp = configure._color_attachment_op.contains(RenderTarget::ATTACHMENT_OP_BIT_STORE) ? SDL_GPU_STOREOP_STORE : SDL_GPU_STOREOP_DONT_CARE;
        for(const auto& [t, cv] : _configure._color_attachments)
            _render_targets.push_back({
                nullptr,
                0,
                0,
                {cv.x(), cv.y(), cv.z(), cv.w()},
                loadOp,
                storeOp
            });
    }

    void draw(GraphicsContext& graphicsContext) override
    {
        for(size_t i = 0; i < _render_targets.size(); ++i)
            _render_targets.at(i).texture = reinterpret_cast<SDL_GPUTexture*>(_configure._color_attachments.at(i)._texture->id());
        if(_configure._depth_stencil_attachment)
            _depth_stencil_target->texture = reinterpret_cast<SDL_GPUTexture*>(_configure._depth_stencil_attachment->id());

        SDL3_GPU_GraphicsContext& gc = ensureGraphicsContext(graphicsContext);
        gc.pushRenderTargets(&_configure, _render_targets, _depth_stencil_target ? &_depth_stencil_target.value() : nullptr);
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
        SDL3_GPU_GraphicsContext& gc = ensureGraphicsContext(graphicsContext);
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
        : _graphics_context(new GraphicsContext(std::move(renderContext), std::move(renderController))), _swapchain_depth_stencil_rt_initial{nullptr, 1.0f, SDL_GPU_LOADOP_CLEAR, SDL_GPU_STOREOP_STORE},
          _swapchain_depth_stencil_rt_blend{nullptr, 0.0f, SDL_GPU_LOADOP_LOAD, SDL_GPU_STOREOP_STORE}
    {
    }

    void onSurfaceCreated() override
    {
    }

    void onSurfaceChanged(const uint32_t width, const uint32_t height) override
    {
        _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));

        const SDL3_GPU_Context& gpuContext = ensureGPUContext(_graphics_context);
        if(_swapchain_depth_stencil_rt_initial.texture)
            SDL_ReleaseGPUTexture(gpuContext._gpu_gevice, _swapchain_depth_stencil_rt_initial.texture);

        const RenderEngineContext::Resolution& resolution = _graphics_context->renderContext()->displayResolution();
        const SDL_GPUTextureCreateInfo textureCreateInfo = {SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREFORMAT_D32_FLOAT, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET, resolution.width, resolution.height, 1, 1};
        SDL_GPUTexture* texture = SDL_CreateGPUTexture(gpuContext._gpu_gevice, &textureCreateInfo);
        _swapchain_depth_stencil_rt_initial.texture = texture;
        _swapchain_depth_stencil_rt_blend.texture = texture;
    }

    void onRenderFrame(const V4& backgroundColor, RenderCommand& renderCommand) override
    {
        const SDL3_GPU_Context& gpuContext = ensureGPUContext(_graphics_context);

        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(gpuContext._gpu_gevice);
        if(!cmdbuf)
        {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return;
        }

        const SDL3_Context& context = _graphics_context->traits().ensure<SDL3_Context>();
        SDL_GPUTexture* swapchainTexture = nullptr;
        if(SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context._main_window, &swapchainTexture, nullptr, nullptr))
        {
            DASSERT(swapchainTexture);
            const Vector<SDL_GPUColorTargetInfo> swapchainRTIInitial = {{
                swapchainTexture,
                0,
                0,
                { backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), backgroundColor.w() },
                SDL_GPU_LOADOP_CLEAR,
                SDL_GPU_STOREOP_STORE
            }};
            const Vector<SDL_GPUColorTargetInfo> swapchainRTIBlend = {{
                swapchainTexture,
                0,
                0,
                {},
                SDL_GPU_LOADOP_LOAD,
                SDL_GPU_STOREOP_STORE
            }};
            SDL3_GPU_GraphicsContext& graphicsContext = ensureGraphicsContext(_graphics_context);
            graphicsContext = {cmdbuf, {nullptr, &swapchainRTIInitial, &_swapchain_depth_stencil_rt_initial}, {nullptr, &swapchainRTIBlend, &_swapchain_depth_stencil_rt_blend}};
            _graphics_context->onDrawFrame();
            renderCommand.draw(_graphics_context);
        }
        else
            SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());

        SDL_SubmitGPUCommandBuffer(cmdbuf);
    }

private:
    op<GraphicsContext> _graphics_context;
    SDL_GPUDepthStencilTargetInfo _swapchain_depth_stencil_rt_initial;
    SDL_GPUDepthStencilTargetInfo _swapchain_depth_stencil_rt_blend;
};

SDL_GPUShaderFormat toGPUShaderFormat(const enums::RenderingBackendBit rendererBackendBit)
{
    if(rendererBackendBit == enums::RENDERING_BACKEND_AUTO)
        return SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
    if(rendererBackendBit == enums::RENDERING_BACKEND_BIT_DIRECT_X)
        return SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL;
    if(rendererBackendBit == enums::RENDERING_BACKEND_BIT_VULKAN)
        return SDL_GPU_SHADERFORMAT_SPIRV;
    CHECK(rendererBackendBit == enums::RENDERING_BACKEND_BIT_METAL, "Unsupported rendering backend: %d", rendererBackendBit);
    return SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB;
}

enums::RendererVersion getRendererVersion(const enums::RenderingBackendBit rendererBackendBit)
{
    if(rendererBackendBit == enums::RENDERING_BACKEND_BIT_DIRECT_X)
        return enums::RENDERER_VERSION_DIRECT_X_12;
    if(rendererBackendBit == enums::RENDERING_BACKEND_BIT_VULKAN)
        return enums::RENDERER_VERSION_VULKAN_13;
    CHECK(rendererBackendBit == enums::RENDERING_BACKEND_BIT_METAL, "Unsupported rendering backend: %d", rendererBackendBit);
    return enums::RENDERER_VERSION_METAL_40;
}

}

RendererFactorySDL3_GPU::RendererFactorySDL3_GPU()
    : RendererFactory({{enums::RENDERING_BACKEND_BIT_DIRECT_X, enums::RENDERING_BACKEND_BIT_METAL}, 16}), _gpu_device(nullptr)
{
}

void RendererFactorySDL3_GPU::onSurfaceCreated(RenderEngine& renderEngine)
{
    const SDL_GPUShaderFormat shaderFormat = toGPUShaderFormat(renderEngine.context()->renderer()._backend);
    _gpu_device = SDL_CreateGPUDevice(
            shaderFormat,
#if ARK_FLAG_BUILD_TYPE
            true,
#else
            false,
#endif
            nullptr);
    CHECK(_gpu_device, "GPUCreateDevice failed: %s", SDL_GetError());

    const SDL3_Context& context = renderEngine.context()->traits().ensure<SDL3_Context>();
    SDL3_GPU_Context& gpuContext = renderEngine.context()->traits().ensure<SDL3_GPU_Context>();
    ASSERT(context._main_window);
    gpuContext._gpu_gevice = _gpu_device;

    const bool success = SDL_ClaimWindowForGPUDevice(_gpu_device, context._main_window);
    CHECK(success, "SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
}

sp<RenderEngineContext> RendererFactorySDL3_GPU::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(0, 0.0f, 1.0f, 1.0f, 0, 1.0f), enums::COORDINATE_SYSTEM_LHS, enums::COORDINATE_SYSTEM_LHS, enums::NDC_DEPTH_RANGE_ZERO_TO_ONE);
    setVersion(renderer._backend == enums::RENDERING_BACKEND_AUTO ? enums::RENDERER_VERSION_VULKAN_13 : getRendererVersion(renderer._backend), renderContext);
    return renderContext;
}

sp<Buffer::Delegate> RendererFactorySDL3_GPU::createBuffer(const Buffer::Usage usage)
{
    if(usage.contains(Buffer::USAGE_BIT_STORAGE))
        return sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE);
    if(usage.contains(Buffer::USAGE_BIT_VERTEX))
        return usage.contains(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_VERTEX) : sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_VERTEX);
    if(usage.contains(Buffer::USAGE_BIT_INDEX))
        return usage.contains(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDEX) : sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDEX);
    if(usage.contains(Buffer::USAGE_BIT_DRAW_INDIRECT))
        return sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDIRECT);
    FATAL("Unknow buffer type: %d", usage.bits());
    return nullptr;
}

sp<RenderTarget> RendererFactorySDL3_GPU::createRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure)
{
    sp<Renderer> fboRenderer = sp<Renderer>::make<OffscreenRendererSDL3_GPU>(renderer, std::move(configure));
    return sp<RenderTarget>::make(std::move(renderer), nullptr, std::move(fboRenderer));
}

sp<PipelineFactory> RendererFactorySDL3_GPU::createPipelineFactory()
{
    return sp<PipelineFactory>::make<PipelineFactorySDL3_GPU>();
}

sp<RenderView> RendererFactorySDL3_GPU::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::make<RenderViewSDL3_GPU>(renderContext, renderController);
}

sp<Texture::Delegate> RendererFactorySDL3_GPU::createTexture(const sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D || parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<Texture::Delegate>::make<TextureSDL3_GPU>(static_cast<uint32_t>(size->widthAsFloat()), static_cast<uint32_t>(size->heightAsFloat()), std::move(parameters));
    return nullptr;
}

}
