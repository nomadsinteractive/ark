#include "sdl3/impl/pipeline_factory/pipeline_factory_sdl3_gpu.h"

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "core/util/uploader_type.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/inf/pipeline.h"
#include "renderer/util/render_util.h"

#include "sdl3/base/context_sdl3_gpu.h"
#include "sdl3/impl/texture/texture_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

namespace {

SDL_GPUShader* createGraphicsShader(SDL_GPUDevice* device, const PipelineLayout& shaderLayout, const StringView source, const enums::ShaderStageBit stageBit)
{
	const SDL_GPUShaderFormat backendFormats = SDL_ShaderCross_GetSPIRVShaderFormats();
	const char* entrypoint = nullptr;
    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
        entrypoint = "main";
    else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
        entrypoint = "main0";
    else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
        entrypoint = "main";
    else
    {
        SDL_Log("%s", "Unrecognized backend shader format!");
        return nullptr;
    }

    const Vector<uint32_t> binaries = RenderUtil::compileSPIR(source, stageBit, enums::RENDERING_BACKEND_BIT_VULKAN, 10);
    const void* bytecode = binaries.data();

    Uint32 samplerCount = 0;
    for(const PipelineLayout::DescriptorSet& i : shaderLayout.samplers().values())
        if(i._stages.has(stageBit))
            ++ samplerCount;

    Uint32 storageTextureCount = 0;
    for(const PipelineLayout::DescriptorSet& i : shaderLayout.images().values())
        if(i._stages.has(stageBit))
            ++ storageTextureCount;

    Uint32 uniformBufferCount = 0;
    for(const PipelineLayout::UBO& i : shaderLayout.ubos())
        if(i._stages.has(stageBit))
            ++ uniformBufferCount;

    Uint32 storageBufferCount = 0;
    for(const PipelineLayout::SSBO& i : shaderLayout.ssbos())
        if(i._stages.has(stageBit))
            ++ storageBufferCount;

    const SDL_ShaderCross_SPIRV_Info spirvInfo = {
        static_cast<const Uint8*>(bytecode),
        binaries.size() * sizeof(uint32_t),
        entrypoint,
        stageBit == enums::SHADER_STAGE_BIT_VERTEX ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT,
#ifdef ARK_FLAG_DEBUG
        true,
#else
        false,
#endif
        nullptr
    };

    SDL_ShaderCross_GraphicsShaderMetadata shaderMetadata = {
        samplerCount,
        storageTextureCount,
        storageBufferCount,
        uniformBufferCount
    };

    SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &spirvInfo, &shaderMetadata);
	if(!shader)
	{
		SDL_Log("Failed to create shader!");
		return nullptr;
	}

	return shader;
}

SDL_GPUBlendFactor toBlendFactor(const PipelineDescriptor::BlendFactor blendFactor, const SDL_GPUBlendFactor defaultBlendFactor)
{
    switch(blendFactor) {
        case PipelineDescriptor::BLEND_FACTOR_ZERO:
            return SDL_GPU_BLENDFACTOR_ZERO;
        case PipelineDescriptor::BLEND_FACTOR_ONE:
            return SDL_GPU_BLENDFACTOR_ONE;
        case PipelineDescriptor::BLEND_FACTOR_SRC_COLOR:
            return SDL_GPU_BLENDFACTOR_SRC_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
            return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_DST_COLOR:
            return SDL_GPU_BLENDFACTOR_DST_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
            return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_SRC_ALPHA:
            return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_DST_ALPHA:
            return SDL_GPU_BLENDFACTOR_DST_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
            return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_CONST_COLOR:
            return SDL_GPU_BLENDFACTOR_CONSTANT_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_CONST_ALPHA:
            return SDL_GPU_BLENDFACTOR_CONSTANT_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_DEFAULT:
            return defaultBlendFactor;
        default:
            break;
    }
    FATAL("Unknow BlendFacor: %d", blendFactor);
    return SDL_GPU_BLENDFACTOR_INVALID;
}

SDL_GPUColorTargetBlendState toColorTargetBlendState(const PipelineDescriptor::TraitBlend& blend)
{
    return {
        toBlendFactor(blend._src_rgb_factor, SDL_GPU_BLENDFACTOR_SRC_ALPHA),
        toBlendFactor(blend._dst_rgb_factor, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA),
        SDL_GPU_BLENDOP_ADD,
        toBlendFactor(blend._src_alpha_factor, SDL_GPU_BLENDFACTOR_SRC_ALPHA),
        toBlendFactor(blend._dst_alpha_factor, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA),
        SDL_GPU_BLENDOP_SUBTRACT,
        0xf,
        blend._enabled,
        false
    };
}

SDL_GPUVertexElementFormat toVertexElementFormat(const Attribute& attribute)
{
    const uint32_t length = attribute.length();
    if(length > 0 && length < 5)
        switch(attribute.type())
        {
            case Attribute::TYPE_FLOAT:
            {
                constexpr SDL_GPUVertexElementFormat formats[4] = {SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4};
                return formats[length - 1];
            }
            case Attribute::TYPE_BYTE:
                case Attribute::TYPE_UBYTE:
                if(length == 2)
                    return attribute.normalized()?  SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM : SDL_GPU_VERTEXELEMENTFORMAT_BYTE2;
                if(length == 4)
                    return attribute.normalized()?  SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM : SDL_GPU_VERTEXELEMENTFORMAT_BYTE4;
                break;
            case Attribute::TYPE_INT:
            {
                constexpr SDL_GPUVertexElementFormat formats[4] = {SDL_GPU_VERTEXELEMENTFORMAT_INT, SDL_GPU_VERTEXELEMENTFORMAT_INT2, SDL_GPU_VERTEXELEMENTFORMAT_INT3, SDL_GPU_VERTEXELEMENTFORMAT_INT4};
                return formats[length - 1];
            }
            case Attribute::TYPE_SHORT:
                if(length == 2)
                    return attribute.normalized() ? SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM : SDL_GPU_VERTEXELEMENTFORMAT_SHORT2;
                if(length == 4)
                    return attribute.normalized() ? SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM : SDL_GPU_VERTEXELEMENTFORMAT_SHORT4;
                break;
            case Attribute::TYPE_USHORT:
                if(length == 2)
                    return attribute.normalized() ? SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM : SDL_GPU_VERTEXELEMENTFORMAT_USHORT2;
                if(length == 4)
                    return attribute.normalized() ? SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM : SDL_GPU_VERTEXELEMENTFORMAT_USHORT4;
                break;
            default:
                break;
        }
    FATAL("Unsupported attribute type: %d length = %d", attribute.type(), length);
    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
}

SDL_GPUPrimitiveType toPrimitiveType(const enums::DrawMode drawMode)
{
    switch(drawMode)
    {
        case enums::DRAW_MODE_LINES:
            return SDL_GPU_PRIMITIVETYPE_LINELIST;
        case enums::DRAW_MODE_POINTS:
            return SDL_GPU_PRIMITIVETYPE_POINTLIST;
        case enums::DRAW_MODE_TRIANGLES:
            return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        case enums::DRAW_MODE_TRIANGLE_STRIP:
            return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
        default:
            break;
    }
    FATAL("Unsupported render mode: %d", drawMode);
    return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
}

void bindUBOSnapshots(SDL_GPUCommandBuffer* cmdbuf, const Vector<RenderBufferSnapshot::UBOSnapshot>& uboSnapshots, const PipelineLayout& shaderLayout, const enums::ShaderStageSet stages)
{
    size_t binding = 0;
    for(const PipelineLayout::UBO& ubo : shaderLayout.ubos())
        if(const enums::ShaderStageSet uboStages = ubo._stages; uboStages & stages)
        {
            DCHECK(binding < uboSnapshots.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), shaderLayout.ubos().size());
            const RenderBufferSnapshot::UBOSnapshot& uboSnapshot = uboSnapshots.at(binding++);
            const void* data = uboSnapshot._buffer.buf();
            const uint32_t size = uboSnapshot._buffer.length();
            if(uboStages.has(enums::SHADER_STAGE_BIT_VERTEX))
                SDL_PushGPUVertexUniformData(cmdbuf, ubo.binding(), data, size);
            if(uboStages.has(enums::SHADER_STAGE_BIT_FRAGMENT))
                SDL_PushGPUFragmentUniformData(cmdbuf, ubo.binding(), data, size);
            if(uboStages.has(enums::SHADER_STAGE_BIT_COMPUTE))
                SDL_PushGPUComputeUniformData(cmdbuf, ubo.binding(), data, size);
        }
}

void setupVertexAttributes(const Attribute& attribute, SDL_GPUVertexAttribute* attribDesc, uint32_t& location, uint32_t& numVertexAttributes)
{
    if(attribute.length() == 16 && attribute.type() == Attribute::TYPE_FLOAT)
        for(uint32_t i = 0; i < 4; ++i)
            attribDesc[numVertexAttributes ++] = {
                location ++,
                attribute.divisor(),
                SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                attribute.offset() + i * 16,
            };
    else
        attribDesc[numVertexAttributes ++] = {
            location ++,
            attribute.divisor(),
            toVertexElementFormat(attribute),
            attribute.offset()
        };
}

class DrawPipelineSDL3_GPU final : public Pipeline {
public:
    DrawPipelineSDL3_GPU(const PipelineBindings& pipelineBindings, String vertexShader, String fragmentShader)
        : _draw_procedure(pipelineBindings.drawProcedure()), _draw_mode(pipelineBindings.drawMode()), _pipeline_descriptor(pipelineBindings.pipelineDescriptor()), _vertex_shader(std::move(vertexShader)), _fragment_shader(std::move(fragmentShader)), _pipeline(nullptr) {
    }

    uint64_t id() override
    {
        return reinterpret_cast<uint64_t>(_pipeline);
    }

    ResourceRecycleFunc recycle() override
    {
        SDL_GPUGraphicsPipeline* pipeline = _pipeline;
        _pipeline = nullptr;

        return [pipeline] (GraphicsContext& graphicsContext) {
            SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
            SDL_ReleaseGPUGraphicsPipeline(gpuDevice, pipeline);
        };
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_pipeline)
        {
            const ContextSDL3_GPU& context = ensureContext(graphicsContext);
            SDL_GPUDevice* gpuDevice = context._gpu_gevice;

            const PipelineLayout& pipelineLayout = _pipeline_descriptor->layout();
            SDL_GPUShader* vertexShader = createGraphicsShader(gpuDevice, pipelineLayout, _vertex_shader, enums::SHADER_STAGE_BIT_VERTEX);
            SDL_GPUShader* fragmentShader = createGraphicsShader(gpuDevice, pipelineLayout, _fragment_shader, enums::SHADER_STAGE_BIT_FRAGMENT);

            SDL_GPUVertexBufferDescription vertexBufferDescription[8];
            Uint32 numVertexBuffers = 0;

            SDL_GPUVertexAttribute vertexAttributes[32];
            Uint32 numVertexAttributes = 0;
            Uint32 location = 0;

            for(const auto& [k, v] : pipelineLayout.streamLayouts())
            {
                vertexBufferDescription[numVertexBuffers] = {
                    numVertexBuffers,
                    v.stride(),
                    k == 0 ? SDL_GPU_VERTEXINPUTRATE_VERTEX : SDL_GPU_VERTEXINPUTRATE_INSTANCE,
                    k
                };
                ++ numVertexBuffers;

                for(const Attribute& i : v.attributes().values())
                    setupVertexAttributes(i, vertexAttributes, location, numVertexAttributes);
            }

            constexpr SDL_GPUColorTargetBlendState defaultBlendState = {
                SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                SDL_GPU_BLENDOP_ADD,
                SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                SDL_GPU_BLENDOP_SUBTRACT,
                0xf,
                true,
                false
            };
            const auto& traits = _pipeline_descriptor->configuration()._traits;
            const SDL_GPUColorTargetBlendState blendState = traits.has(PipelineDescriptor::TRAIT_TYPE_BLEND) ? toColorTargetBlendState(traits.at(PipelineDescriptor::TRAIT_TYPE_BLEND)._blend) : defaultBlendState;

            Uint32 numColorTargets = 0;
            SDL_GPUColorTargetDescription colorTargetDescriptions[8];

            bool hasDepthStencilTarget = false;
            SDL_GPUTextureFormat depthStencilFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
            const GraphicsContextSDL3_GPU& gc = ensureGraphicsContext(graphicsContext);
            if(const RenderTargetContext& renderTarget = gc.renderTarget(); renderTarget._create_config)
            {
                const RenderTarget::Configure& rtCreateConfig = *renderTarget._create_config;
                for(const auto& [t, cv] : rtCreateConfig._color_attachments)
                {
                    colorTargetDescriptions[numColorTargets] = {
                        t->delegate().cast<TextureSDL3_GPU>()->textureFormat(),
                        numColorTargets == 0 ? blendState : SDL_GPUColorTargetBlendState{}
                    };
                    ++ numColorTargets;
                }
            }
            else
            {
                colorTargetDescriptions[0] = {
                    SDL_GetGPUSwapchainTextureFormat(gpuDevice, context._main_window),
                    blendState
                };
                numColorTargets = 1;
            }
            const SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
                vertexShader,
                fragmentShader,
                {
                    vertexBufferDescription,
                    numVertexBuffers,
                    vertexAttributes,
                    numVertexAttributes
                },
                toPrimitiveType(_draw_mode), {
                    SDL_GPU_FILLMODE_FILL,
                    SDL_GPU_CULLMODE_BACK,
                    SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
                }, {
                    SDL_GPU_SAMPLECOUNT_1
                }, {
                    SDL_GPU_COMPAREOP_LESS_OR_EQUAL
                }
                , {
                    colorTargetDescriptions,
                    numColorTargets,
                    depthStencilFormat,
                    hasDepthStencilTarget
                }
            };
            _pipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineCreateInfo);
            CHECK(_pipeline, "%s", SDL_GetError());

            SDL_ReleaseGPUShader(gpuDevice, vertexShader);
            SDL_ReleaseGPUShader(gpuDevice, fragmentShader);
        }
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DASSERT(drawingContext._vertices);
        DASSERT(drawingContext._indices);

        const PipelineBindings& pipelineBindings = drawingContext._bindings;
        const GraphicsContextSDL3_GPU& sdl3GC = ensureGraphicsContext(graphicsContext);

        const RenderTargetContext& renderTargets = sdl3GC.renderTarget();
        const Optional<SDL_GPUDepthStencilTargetInfo>& depthStencilTarget = renderTargets._depth_stencil_target;
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(sdl3GC._command_buffer, renderTargets._color_targets.data(), renderTargets._color_targets.size(), depthStencilTarget ? depthStencilTarget.get() : nullptr);

        SDL_BindGPUGraphicsPipeline(renderPass, _pipeline);

        constexpr enums::ShaderStageSet currentStageSets = {enums::SHADER_STAGE_BIT_VERTEX, enums::SHADER_STAGE_BIT_FRAGMENT};
        bindUBOSnapshots(sdl3GC._command_buffer, drawingContext._buffer_snapshot->_ubos, drawingContext._bindings->pipelineLayout(), currentStageSets);

        const SDL_GPUBufferBinding vertexBufferBinding = {reinterpret_cast<SDL_GPUBuffer*>(drawingContext._vertices.id()), 0};
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

        const SDL_GPUBufferBinding indexBufferBinding = {reinterpret_cast<SDL_GPUBuffer*>(drawingContext._indices.id()), 0};
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, sizeof(element_index_t) == 2 ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT);

        SDL_GPUTextureSamplerBinding textureSamplerBinding[8];
        Uint32 samplerCount = 0;
        ASSERT(pipelineBindings.samplers().size() < 8);
        for(const auto& [_, k, v] : pipelineBindings.samplers())
        {
            TextureSDL3_GPU& texture = k->delegate().cast<TextureSDL3_GPU>();
            if(!texture.texture())
                return SDL_EndGPURenderPass(renderPass);

            textureSamplerBinding[samplerCount ++] = {
                texture.texture(),
                texture.ensureSampler(ensureGPUDevice(graphicsContext))
            };
        }
        SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBinding, samplerCount);

        switch(_draw_procedure)
        {
            case enums::DRAW_PROCEDURE_DRAW_ELEMENTS:
                SDL_DrawGPUIndexedPrimitives(renderPass, drawingContext._draw_count, 1, 0, 0, 0);
                break;
            case enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT: {
                const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();
                SDL_DrawGPUIndexedPrimitivesIndirect(renderPass, reinterpret_cast<SDL_GPUBuffer*>(param._indirect_cmds.id()), 0, param._indirect_cmd_count);
                break;
            }
            default:
                break;
        }

        SDL_EndGPURenderPass(renderPass);
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        DFATAL("Shouldn't be here");
    }

private:
    enums::DrawProcedure _draw_procedure;
    enums::DrawMode _draw_mode;

    sp<PipelineDescriptor> _pipeline_descriptor;
    String _vertex_shader;
    String _fragment_shader;

    SDL_GPUGraphicsPipeline* _pipeline;
};

class ComputePipelineSDL3_GPU final : public Pipeline {
public:
    ComputePipelineSDL3_GPU(const PipelineDescriptor& pipelineDescriptor, String computeShader)
        : _pipeline_descriptor(pipelineDescriptor), _compute_shader(std::move(computeShader)) {
    }

    uint64_t id() override
    {
        return reinterpret_cast<uint64_t>(_pipeline);
    }

    ResourceRecycleFunc recycle() override
    {
        SDL_GPUComputePipeline* pipeline = _pipeline;
        return [pipeline] (GraphicsContext& graphicsContext) {
            SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
            SDL_ReleaseGPUComputePipeline(gpuDevice, pipeline);
        };
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_pipeline)
        {
            SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

            const SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(gpuDevice);
            const char *entrypoint;
            if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
                entrypoint = "main";
            else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
                entrypoint = "main0";
            else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
                entrypoint = "main";
            else
            {
                SDL_Log("%s", "Unrecognized backend shader format!");
                return;
            }

            const Vector<uint32_t> binaries = RenderUtil::compileSPIR(_compute_shader, enums::SHADER_STAGE_BIT_COMPUTE, enums::RENDERING_BACKEND_BIT_VULKAN, 10);
            const void* bytecode = binaries.data();

            const SDL_ShaderCross_SPIRV_Info spirvInfo = {
                static_cast<const Uint8*>(bytecode),
                binaries.size() * sizeof(uint32_t),
                entrypoint,
                SDL_SHADERCROSS_SHADERSTAGE_COMPUTE,
        #ifdef ARK_FLAG_DEBUG
                true,
        #else
                false,
        #endif
                nullptr
            };

            const PipelineLayout& shaderLayout = _pipeline_descriptor.layout();
            Uint32 samplerCount = 0;
            for(const PipelineLayout::DescriptorSet& i : shaderLayout.samplers().values())
                if(i._stages.has(enums::SHADER_STAGE_BIT_COMPUTE))
                    ++ samplerCount;

            Uint32 storageTextureCount = 0;
            for(const PipelineLayout::DescriptorSet& i : shaderLayout.images().values())
                if(i._stages.has(enums::SHADER_STAGE_BIT_COMPUTE))
                    ++ storageTextureCount;

            Uint32 uniformBufferCount = 0;
            for(const PipelineLayout::UBO& i : shaderLayout.ubos())
                if(i._stages.has(enums::SHADER_STAGE_BIT_COMPUTE))
                    ++ uniformBufferCount;

            Uint32 storageBufferCount = 0;
            for(const PipelineLayout::SSBO& i : shaderLayout.ssbos())
                if(i._stages.has(enums::SHADER_STAGE_BIT_COMPUTE))
                    ++ storageBufferCount;

            SDL_ShaderCross_ComputePipelineMetadata shaderMetadata = {
                samplerCount,
                storageTextureCount,
                storageBufferCount,
                0,
                0,
                uniformBufferCount
            };

            _pipeline = SDL_ShaderCross_CompileComputePipelineFromSPIRV(gpuDevice, &spirvInfo, &shaderMetadata);
            CHECK(_pipeline, "%s", SDL_GetError());
        }
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DFATAL("Shouldn't be here");
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        const GraphicsContextSDL3_GPU& sdl3GC = ensureGraphicsContext(graphicsContext);

        SDL_GPUStorageTextureReadWriteBinding storageTextureBindings[8];
        SDL_GPUStorageBufferReadWriteBinding storageBufferBindings[8];

        SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(sdl3GC._command_buffer, storageTextureBindings, 1, storageBufferBindings, 0);
        SDL_BindGPUComputePipeline(computePass, _pipeline);
        SDL_DispatchGPUCompute(computePass, computeContext._num_work_groups[0], computeContext._num_work_groups[1], computeContext._num_work_groups[2]);
        SDL_EndGPUComputePass(computePass);
    }

private:
    PipelineDescriptor _pipeline_descriptor;
    String _compute_shader;

    SDL_GPUComputePipeline* _pipeline;
};

}

sp<Pipeline> PipelineFactorySDL3_GPU::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, std::map<enums::ShaderStageBit, String> stages)
{
    const sp<PipelineDescriptor>& pipelineDescriptor = pipelineBindings.pipelineDescriptor();
    if(const auto vIter = stages.find(enums::SHADER_STAGE_BIT_VERTEX); vIter != stages.end())
    {
        const auto fIter = stages.find(enums::SHADER_STAGE_BIT_FRAGMENT);
        CHECK(fIter != stages.end(), "Pipeline has no fragment shader(only vertex shader available)");
        return sp<Pipeline>::make<DrawPipelineSDL3_GPU>(pipelineBindings, std::move(vIter->second), std::move(fIter->second));
    }
    const auto cIter = stages.find(enums::SHADER_STAGE_BIT_COMPUTE);
    CHECK(cIter != stages.end(), "Pipeline has no compute shader");
    return sp<Pipeline>::make<ComputePipelineSDL3_GPU>(pipelineDescriptor, std::move(cIter->second));
}

}
