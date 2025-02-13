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
#include "sdl3/impl/pipeline_factory/pipeline_factory_sdl3_gpu.h"
#include "sdl3/impl/texture/texture_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

namespace {

class SnippetSDL3 final : public Snippet {
public:
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override {
        const String sLocation = "location";
        const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;

        RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);

        const PipelineInput& pipelineInput = pipelineLayout.input();
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
            const Vector<String>& samplerNames = fragment->_declaration_samplers.vars().keys();
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
        return sp<Snippet>::make<SnippetSDL3>();
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
            false,
            nullptr);
    if(!_gpu_device)
    {
        SDL_Log("GPUCreateDevice failed");
        return;
    }

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

class BufferSDL3 final : public Buffer::Delegate {
public:
    BufferSDL3(const SDL_GPUBufferUsageFlags usageFlags)
        : _usage_flags(usageFlags) {
    }

    uint64_t id() override
    {
        return reinterpret_cast<uint64_t>(_buffer);
    }

    void upload(GraphicsContext& graphicsContext) override
    {
    }

    ResourceRecycleFunc recycle() override
    {
        SDL_GPUBuffer* buffer = _buffer;
        _buffer = nullptr;

        return [buffer] (GraphicsContext& graphicsContext) {
            SDL_ReleaseGPUBuffer(ensureGPUDevice(graphicsContext), buffer);
        };
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

        const Uint32 inputSize = input.size();
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, inputSize};
        SDL_GPUTransferBuffer* uploadTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

        void* transferData = SDL_MapGPUTransferBuffer(gpuDevice, uploadTransferBuffer, false);
        UploaderType::writeTo(input, transferData);
        SDL_UnmapGPUTransferBuffer(gpuDevice, uploadTransferBuffer);

        SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

        const SDL_GPUTransferBufferLocation transferBufferLocation{uploadTransferBuffer, 0};
        const SDL_GPUBufferRegion bufferRegion{_buffer, 0, inputSize};
        SDL_UploadToGPUBuffer(copyPass, &transferBufferLocation, &bufferRegion, false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        SDL_ReleaseGPUTransferBuffer(gpuDevice, uploadTransferBuffer);
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, static_cast<Uint32>(size)};
        SDL_GPUTransferBuffer* downloadTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

        SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

        const SDL_GPUBufferRegion bufferRegion{_buffer, static_cast<Uint32>(offset), static_cast<Uint32>(size)};
        const SDL_GPUTransferBufferLocation transferBufferLocation{downloadTransferBuffer, 0};
        SDL_DownloadFromGPUBuffer(copyPass, &bufferRegion, &transferBufferLocation);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

        const Uint8* downloadedData = static_cast<const Uint8*>(SDL_MapGPUTransferBuffer(gpuDevice, downloadTransferBuffer, false));
        memcpy(ptr, downloadedData, size);
        SDL_UnmapGPUTransferBuffer(gpuDevice, downloadTransferBuffer);

        SDL_ReleaseGPUTransferBuffer(gpuDevice, downloadTransferBuffer);
    }

private:
    SDL_GPUBufferUsageFlags _usage_flags;
    SDL_GPUBuffer* _buffer;
};

sp<Buffer::Delegate> RendererFactorySDL3_GPU::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    switch(type)
    {
        case Buffer::TYPE_VERTEX:
            return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3>(SDL_GPU_BUFFERUSAGE_VERTEX) : sp<Buffer::Delegate>::make<BufferSDL3>(SDL_GPU_BUFFERUSAGE_VERTEX);
        case Buffer::TYPE_INDEX:
            return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<BufferSDL3>(SDL_GPU_BUFFERUSAGE_INDEX) : sp<Buffer::Delegate>::make<BufferSDL3>(SDL_GPU_BUFFERUSAGE_INDEX);
        case Buffer::TYPE_DRAW_INDIRECT:
            return sp<Buffer::Delegate>::make<BufferSDL3>(SDL_GPU_BUFFERUSAGE_INDIRECT);
        case Buffer::TYPE_STORAGE:
            return sp<Buffer::Delegate>::make<BufferSDL3>(SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE);
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

class RenderViewSDL3 final : public RenderView {
public:
    RenderViewSDL3(sp<RenderEngineContext> renderContext, sp<RenderController> renderController)
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
        ContextSDL3_GPU& context = _graphics_context->traits().ensure<ContextSDL3_GPU>();

        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context._gpu_gevice);
        if(!cmdbuf)
        {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return;
        }

        if(!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context._main_window, &context._swapchain_texture, nullptr, nullptr))
        {
            SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            return;
        }

        if(context._swapchain_texture)
        {
            SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
            colorTargetInfo.texture = context._swapchain_texture;
            colorTargetInfo.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
            colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, nullptr);

            // SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
            // SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ .buffer = VertexBuffer, .offset = 0 }, 1);
            // SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ .buffer = IndexBuffer, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
            // SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = Texture, .sampler = Samplers[CurrentSamplerIndex] }, 1);
            // SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);

            SDL_EndGPURenderPass(renderPass);

            _graphics_context->onDrawFrame();
            renderCommand.draw(_graphics_context);
        }

        SDL_SubmitGPUCommandBuffer(cmdbuf);
    }

private:
    op<GraphicsContext> _graphics_context;
};

sp<RenderView> RendererFactorySDL3_GPU::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::make<RenderViewSDL3>(renderContext, renderController);
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
