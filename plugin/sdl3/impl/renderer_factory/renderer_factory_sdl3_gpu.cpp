#include "sdl3/impl/renderer_factory/renderer_factory_sdl3_gpu.h"

#include "core/util/uploader_type.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"
#include "graphics/inf/render_command.h"
#include "graphics/inf/render_view.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
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
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override {
        const String sLocation = "location";
        const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;

        RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);

        const ShaderLayout& pipelineInput = pipelineLayout.input();
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
        {
            RenderUtil::setLayoutDescriptor(vertex->_declaration_images, "binding", static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size() + pipelineInput.samplerCount()));
            vertex->_predefined_macros.emplace_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
        {
            fragment->linkNextStage("FragColor");
            const uint32_t bindingOffset = std::max<uint32_t>(2, pipelineInput.ubos().size() + pipelineInput.ssbos().size());
            RenderUtil::setLayoutDescriptor(fragment->_declaration_images, "binding", bindingOffset + static_cast<uint32_t>(fragment->_declaration_samplers.vars().size()));

            uint32_t binding = 0;
            constexpr uint32_t samplerOffset = 16;
            const std::vector<String> samplerNames = fragment->_declaration_samplers.vars().keys();
            fragment->_declaration_samplers.clear();
            for(const String& k : samplerNames)
            {
                fragment->_declaration_samplers.declare("sampler", "", k + "S", Strings::sprintf("binding = %d", bindingOffset + samplerOffset + binding));
                fragment->_declaration_samplers.declare("texture2D", "", k + "T", Strings::sprintf("binding = %d", bindingOffset + binding++));
                fragment->_predefined_macros.emplace_back(Strings::sprintf("#define %s sampler2D(%sT, %sS)", k.c_str(), k.c_str(), k.c_str()));
            }
        }

        if(const ShaderPreprocessor* compute = context.computingStage().get())
        {
            const uint32_t bindingOffset = static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size());
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

        for(const auto& [_, v] : context.renderStages())
        {
            ShaderPreprocessor& preprocessor = v;
            preprocessor._version = 450;
            preprocessor.declareUBOStruct(pipelineInput);
            preprocessor._predefined_macros.push_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor._predefined_macros.push_back("#extension GL_ARB_shading_language_420pack : enable");
        }
    }

    sp<DrawEvents> makeDrawEvents() override {
        return sp<Snippet::DrawEvents>::make();
    }

};

class SnippetFactorySDL3 final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override
    {
        return sp<Snippet>::make<SnippetSDL3_GPU>();
    }
};

void setVersion(Ark::RendererVersion version, RenderEngineContext& vkContext)
{
    LOGD("Choose Renderer Version = %d", version);
    Map<String, String>& definitions = vkContext.definitions();

    definitions["vert.in"] = "in";
    definitions["vert.out"] = "out";
    definitions["frag.in"] = "in";
    definitions["frag.out"] = "out";
    definitions["frag.color"] = "f_FragColor";
    definitions["camera.uVP"] = "u_viewProj";
    definitions["camera.uView"] = "u_view";
    definitions["camera.uProjection"] = "u_proj";
    vkContext.setSnippetFactory(sp<SnippetFactorySDL3>::make());

    vkContext.setVersion(version);
}

}

RendererFactorySDL3_GPU::RendererFactorySDL3_GPU()
    : RendererFactory({Ark::COORDINATE_SYSTEM_RHS, false, 16}), _gpu_device(nullptr)
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

    if(!SDL_ClaimWindowForGPUDevice(_gpu_device, sdl3GPUContext._main_window))
    {
        SDL_Log("GPUClaimWindow failed");
        return;
    }
    sdl3GPUContext._gpu_gevice = _gpu_device;
}

sp<RenderEngineContext> RendererFactorySDL3_GPU::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    setVersion(renderer._version == Ark::RENDERER_VERSION_AUTO ? Ark::RENDERER_VERSION_VULKAN_13 : renderer._version, renderContext);
    return renderContext;
}

sp<Buffer::Delegate> RendererFactorySDL3_GPU::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    switch(type)
    {
        case Buffer::TYPE_VERTEX:
            return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_VERTEX) : sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_VERTEX);
        case Buffer::TYPE_INDEX:
            return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDEX) : sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDEX);
        case Buffer::TYPE_DRAW_INDIRECT:
            return sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_INDIRECT);
        case Buffer::TYPE_STORAGE:
            return sp<Buffer::Delegate>::make<BufferSDL3_GPU>(SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE);
        default:
            FATAL("Unknow buffer type: %d", type);
            break;
    }
    return nullptr;
}

sp<Camera::Delegate> RendererFactorySDL3_GPU::createCamera(const Ark::RendererCoordinateSystem rcs)
{
    return rcs == Ark::COORDINATE_SYSTEM_LHS ? sp<Camera::Delegate>::make<Camera::DelegateLH_NO>() :  sp<Camera::Delegate>::make<Camera::DelegateRH_NO>();
}

sp<RenderTarget> RendererFactorySDL3_GPU::createRenderTarget(sp<Renderer> renderer, RenderTarget::CreateConfigure configure)
{
    return nullptr;
}

sp<PipelineFactory> RendererFactorySDL3_GPU::createPipelineFactory()
{
    return sp<PipelineFactory>::make<PipelineFactorySDL3_GPU>();
}

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

    void onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand) override
    {
        const ContextSDL3_GPU& context = _graphics_context->traits().ensure<ContextSDL3_GPU>();
        GraphicsContextSDL3_GPU& graphicsContext = _graphics_context->traits().ensure<GraphicsContextSDL3_GPU>();

        graphicsContext._command_buffer = SDL_AcquireGPUCommandBuffer(context._gpu_gevice);
        if(!graphicsContext._command_buffer)
        {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return;
        }

        graphicsContext._color_target_info = {
            nullptr,
            0,
            0,
            { 0.0f, 0.0f, 0.0f, 1.0f },
            SDL_GPU_LOADOP_CLEAR,
            SDL_GPU_STOREOP_STORE
        };

        if(!SDL_WaitAndAcquireGPUSwapchainTexture(graphicsContext._command_buffer, context._main_window, &graphicsContext._color_target_info.texture, nullptr, nullptr))
        {
            SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            return;
        }

        if(graphicsContext._color_target_info.texture)
        {
            _graphics_context->onDrawFrame();
            renderCommand.draw(_graphics_context);
        }

        SDL_SubmitGPUCommandBuffer(graphicsContext._command_buffer);
    }

private:
    op<GraphicsContext> _graphics_context;
};

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
