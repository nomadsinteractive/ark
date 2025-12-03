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

#include "sdl3/base/sdl3_context.h"
#include "sdl3/base/context_sdl3_gpu.h"
#include "sdl3/impl/texture/texture_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

namespace {

SDL_GPUShader* createGraphicsShader(SDL_GPUDevice* device, const StringView sourceView, const enums::ShaderStageBit stageBit)
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

    const String sourceStr = sourceView;
    const Vector<uint32_t> binaries = RenderUtil::compileSPIR(sourceStr, stageBit, enums::RENDERING_BACKEND_BIT_VULKAN, 10);
    const void* bytecode = binaries.data();
    const SDL_ShaderCross_SPIRV_Info spirvInfo = {
        static_cast<const Uint8*>(bytecode),
        binaries.size() * sizeof(uint32_t),
        entrypoint,
        stageBit == enums::SHADER_STAGE_BIT_VERTEX ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT
    };

    const SDL_ShaderCross_GraphicsShaderMetadata* shaderMetadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirvInfo.bytecode, spirvInfo.bytecode_size, 0);
    SDL_ClearError();
    SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &spirvInfo, &shaderMetadata->resource_info, 0);
	if(const char* lastError = SDL_GetError(); !shader || lastError[0])
	{
        FATAL("%s\n\nFailed to create shader, SDL Error: %s", sourceStr.c_str(), lastError);
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
            case Attribute::TYPE_UINT:
            {
                constexpr SDL_GPUVertexElementFormat formats[4] = {SDL_GPU_VERTEXELEMENTFORMAT_UINT, SDL_GPU_VERTEXELEMENTFORMAT_UINT2, SDL_GPU_VERTEXELEMENTFORMAT_UINT3, SDL_GPU_VERTEXELEMENTFORMAT_UINT4};
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

SDL_GPUCompareOp toCompareOp(const PipelineDescriptor::CompareFunc compareOp)
{
    constexpr SDL_GPUCompareOp sdlCompareOps[] = {SDL_GPU_COMPAREOP_LESS_OR_EQUAL, SDL_GPU_COMPAREOP_ALWAYS, SDL_GPU_COMPAREOP_NEVER, SDL_GPU_COMPAREOP_EQUAL, SDL_GPU_COMPAREOP_NOT_EQUAL, SDL_GPU_COMPAREOP_LESS,
                                                  SDL_GPU_COMPAREOP_GREATER, SDL_GPU_COMPAREOP_LESS_OR_EQUAL, SDL_GPU_COMPAREOP_GREATER_OR_EQUAL};
    DASSERT(compareOp < PipelineDescriptor::COMPARE_FUNC_LENGTH);
    return sdlCompareOps[compareOp];
}

SDL_GPUStencilOp toStencilOp(const PipelineDescriptor::StencilFunc stencilOp)
{
    constexpr SDL_GPUStencilOp sdlStencilOps[] = {SDL_GPU_STENCILOP_KEEP, SDL_GPU_STENCILOP_ZERO, SDL_GPU_STENCILOP_REPLACE, SDL_GPU_STENCILOP_INCREMENT_AND_CLAMP, SDL_GPU_STENCILOP_INCREMENT_AND_WRAP,
                                                  SDL_GPU_STENCILOP_DECREMENT_AND_CLAMP, SDL_GPU_STENCILOP_DECREMENT_AND_WRAP, SDL_GPU_STENCILOP_INVERT};
    DASSERT(stencilOp < PipelineDescriptor::STENCIL_FUNC_LENGTH);
    return sdlStencilOps[stencilOp];

}

SDL_GPUDepthStencilState toDepthStencilState(const PipelineDescriptor& pipelineDescriptor)
{
    SDL_GPUDepthStencilState depthStencilState = {SDL_GPU_COMPAREOP_LESS_OR_EQUAL};
    depthStencilState.enable_depth_test = true;
    depthStencilState.enable_depth_write = true;
    if(const PipelineDescriptor::TraitDepthTest* depthTest = pipelineDescriptor.getTrait<PipelineDescriptor::TraitDepthTest>())
    {
        depthStencilState.enable_depth_test = depthTest->_enabled;
        depthStencilState.enable_depth_write = depthTest->_write_enabled;
        depthStencilState.compare_op = toCompareOp(depthTest->_func);
    }
    if(const PipelineDescriptor::TraitStencilTest* stencilTest = pipelineDescriptor.getTrait<PipelineDescriptor::TraitStencilTest>())
    {
        depthStencilState.enable_stencil_test = true;
        depthStencilState.write_mask = 0xff;
        depthStencilState.front_stencil_state.fail_op = toStencilOp(stencilTest->_front._op_dpass);
        depthStencilState.front_stencil_state.pass_op = toStencilOp(stencilTest->_front._op);
        depthStencilState.front_stencil_state.depth_fail_op = toStencilOp(stencilTest->_front._op_dfail);
        depthStencilState.front_stencil_state.compare_op = toCompareOp(stencilTest->_front._func);
        depthStencilState.back_stencil_state.fail_op = toStencilOp(stencilTest->_back._op_dpass);
        depthStencilState.back_stencil_state.pass_op = toStencilOp(stencilTest->_back._op);
        depthStencilState.back_stencil_state.depth_fail_op = toStencilOp(stencilTest->_back._op_dfail);
        depthStencilState.back_stencil_state.compare_op = toCompareOp(stencilTest->_back._func);
    }
    return depthStencilState;
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
            if(uboStages.contains(enums::SHADER_STAGE_BIT_VERTEX))
                SDL_PushGPUVertexUniformData(cmdbuf, ubo.binding(), data, size);
            if(uboStages.contains(enums::SHADER_STAGE_BIT_FRAGMENT))
                SDL_PushGPUFragmentUniformData(cmdbuf, ubo.binding(), data, size);
            if(uboStages.contains(enums::SHADER_STAGE_BIT_COMPUTE))
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
            const SDL3_Context& sdl3Context = graphicsContext.traits().ensure<SDL3_Context>();
            const SDL3_GPU_Context& context = ensureGPUContext(graphicsContext);
            SDL_GPUDevice* gpuDevice = context._gpu_gevice;

            const PipelineLayout& pipelineLayout = _pipeline_descriptor->layout();
            SDL_GPUShader* vertexShader = createGraphicsShader(gpuDevice, _vertex_shader, enums::SHADER_STAGE_BIT_VERTEX);
            SDL_GPUShader* fragmentShader = createGraphicsShader(gpuDevice, _fragment_shader, enums::SHADER_STAGE_BIT_FRAGMENT);

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
                    0
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
            const PipelineDescriptor::TraitBlend* blend = _pipeline_descriptor->getTrait<PipelineDescriptor::TraitBlend>();
            const SDL_GPUColorTargetBlendState blendState = blend ? toColorTargetBlendState(*blend) : defaultBlendState;

            Uint32 numColorTargets = 0;
            SDL_GPUColorTargetDescription colorTargetDescriptions[8];

            bool hasDepthStencilTarget = false;
            SDL_GPUTextureFormat depthStencilFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
            SDL3_GPU_GraphicsContext& gc = ensureGraphicsContext(graphicsContext);
            if(const RenderTargetContext& renderTarget = gc.getCurrentRenderTarget(); renderTarget._create_config)
            {
                const RenderTarget::Configure& rtCreateConfig = *renderTarget._create_config;
                CHECK(rtCreateConfig._color_attachments.size() <= 8, "Rendertarget now can only hold no more than 8 color attachments.");
                for(const auto& [t, cv] : rtCreateConfig._color_attachments)
                {
                    const bool isIntegerTarget = t->parameters()->_format.contains(Texture::FORMAT_INTEGER);
                    colorTargetDescriptions[numColorTargets++] = {
                        t->delegate().cast<TextureSDL3_GPU>()->textureFormat(),
                        isIntegerTarget ? SDL_GPUColorTargetBlendState{} : blendState
                    };
                }
                if(rtCreateConfig._depth_stencil_attachment)
                {
                    const auto delegate = rtCreateConfig._depth_stencil_attachment->delegate().cast<TextureSDL3_GPU>();
                    depthStencilFormat = delegate->textureFormat();
                    hasDepthStencilTarget = true;
                }
            }
            else
            {
                colorTargetDescriptions[0] = {
                    SDL_GetGPUSwapchainTextureFormat(gpuDevice, sdl3Context._main_window),
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
                }, toDepthStencilState(_pipeline_descriptor)
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

        SDL3_GPU_GraphicsContext& sdl3GC = ensureGraphicsContext(graphicsContext);

        const RenderTargetContext& renderTargets = sdl3GC.getCurrentRenderTarget();
        const Optional<SDL_GPUDepthStencilTargetInfo>& depthStencilTarget = *renderTargets._depth_stencil_target;
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(sdl3GC._command_buffer, renderTargets._color_targets->data(), renderTargets._color_targets->size(), depthStencilTarget ? depthStencilTarget.get() : nullptr);

        SDL_BindGPUGraphicsPipeline(renderPass, _pipeline);

        constexpr enums::ShaderStageSet currentStageSets = {enums::SHADER_STAGE_BIT_VERTEX, enums::SHADER_STAGE_BIT_FRAGMENT};
        bindUBOSnapshots(sdl3GC._command_buffer, drawingContext._buffer_snapshot->_ubos, drawingContext._bindings->pipelineLayout(), currentStageSets);

        const SDL_GPUBufferBinding vertexBufferBinding = {reinterpret_cast<SDL_GPUBuffer*>(drawingContext._vertices.id()), 0};
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

        const SDL_GPUBufferBinding indexBufferBinding = {reinterpret_cast<SDL_GPUBuffer*>(drawingContext._indices.id()), 0};
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, sizeof(element_index_t) == 2 ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT);

        const PipelineBindings& pipelineBindings = drawingContext._bindings;
        const PipelineLayout& pipelineLayout = pipelineBindings.pipelineLayout();
        if(!pipelineBindings.samplers().empty())
        {
            Uint32 samplerCount = 0;
            SDL_GPUTextureSamplerBinding textureSamplerBinding[8];
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
        }

        if(!pipelineLayout.ssbos().empty())
        {
            SDL_GPUBuffer* storageBuffers[8] = {};
            uint32_t storageBufferCount = 0;
            ASSERT(pipelineLayout.ssbos().size() < 8);
            for(const PipelineLayout::SSBO& i : pipelineLayout.ssbos())
                if(i._stages.contains(enums::SHADER_STAGE_BIT_FRAGMENT))
                    storageBuffers[storageBufferCount ++] = reinterpret_cast<SDL_GPUBuffer*>(i._buffer.id());
            SDL_BindGPUFragmentStorageBuffers(renderPass, 0, storageBuffers, storageBufferCount);
        }

        if(!pipelineBindings.images().empty())
        {
            SDL_GPUTexture* storageTextures[8] = {};
            uint32_t storageTextureCount = 0;
            ASSERT(pipelineBindings.images().size() < 8);
            for(const PipelineDescriptor::BindedTexture& i : pipelineBindings.images())
                if(i._descriptor_set._stages.contains(enums::SHADER_STAGE_BIT_FRAGMENT))
                    storageTextures[storageTextureCount ++] = reinterpret_cast<SDL_GPUTexture*>(i._texture->id());
            SDL_BindGPUFragmentStorageTextures(renderPass, 0, storageTextures, storageTextureCount);
        }

        switch(_draw_procedure)
        {
            case enums::DRAW_PROCEDURE_DRAW_ELEMENTS:
                SDL_DrawGPUIndexedPrimitives(renderPass, drawingContext._draw_count, 1, 0, 0, 0);
                break;
            case enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT: {
                const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();
                SDL_GPUBufferBinding instanceBuffers[8] = {};
                uint32_t numInstanceBuffers = 0;
                for(const auto& [i, j] : param._instance_buffer_snapshots)
                {
                    j.upload(graphicsContext);
                    const auto buffer = reinterpret_cast<SDL_GPUBuffer*>(j.id());
                    CHECK(buffer, "Invaild Instanced Array Buffer: %d", i);
                    instanceBuffers[numInstanceBuffers ++] = {buffer, 0};
                }
                param._indirect_cmds.upload(graphicsContext);
                SDL_BindGPUVertexBuffers(renderPass, 1, instanceBuffers, numInstanceBuffers);
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
        : _pipeline_descriptor(pipelineDescriptor), _compute_shader(std::move(computeShader)), _pipeline(nullptr) {
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
                SDL_SHADERCROSS_SHADERSTAGE_COMPUTE
            };

            const SDL_ShaderCross_ComputePipelineMetadata* computeMetadata = SDL_ShaderCross_ReflectComputeSPIRV(spirvInfo.bytecode, spirvInfo.bytecode_size, 0);
            SDL_ClearError();
            _pipeline = SDL_ShaderCross_CompileComputePipelineFromSPIRV(gpuDevice, &spirvInfo, computeMetadata, 0);
            CHECK(_pipeline, "%s", SDL_GetError());
        }
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DFATAL("Shouldn't be here");
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        const SDL3_GPU_GraphicsContext& sdl3GC = ensureGraphicsContext(graphicsContext);

        uint32_t numReadWriteStorageTextures = 0;
        uint32_t numReadOnlyStorageTextures = 0;
        SDL_GPUStorageTextureReadWriteBinding storageTextureBindings[16];
        SDL_GPUTexture* readonlyStorageTextures[16];
        for(const PipelineDescriptor::BindedTexture& i : computeContext._bindings->images())
            if(i._descriptor_set._stages.contains(enums::SHADER_STAGE_BIT_COMPUTE))
            {
                if(i._descriptor_set._binding._qualifier.contains(enums::SHADER_TYPE_QUALIFIER_READONLY))
                    readonlyStorageTextures[numReadOnlyStorageTextures++] = reinterpret_cast<SDL_GPUTexture*>(i._texture->id());
                else
                    storageTextureBindings[numReadWriteStorageTextures++] = {reinterpret_cast<SDL_GPUTexture*>(i._texture->id()), 0, 0};
            }

        uint32_t numReadWriteStorageBuffers = 0;
        uint32_t numReadOnlyStorageBuffers = 0;
        SDL_GPUStorageBufferReadWriteBinding storageBufferBindings[16];
        SDL_GPUBuffer* readonlyStorageBuffers[16];
        for(const PipelineLayout::SSBO& i : computeContext._bindings->pipelineLayout()->ssbos())
            if(i._stages.contains(enums::SHADER_STAGE_BIT_COMPUTE))
            {
                if(i._binding._qualifier.contains(enums::SHADER_TYPE_QUALIFIER_READONLY))
                    readonlyStorageBuffers[numReadOnlyStorageBuffers++] = reinterpret_cast<SDL_GPUBuffer*>(i._buffer.id());
                else
                    storageBufferBindings[numReadWriteStorageBuffers++] = {reinterpret_cast<SDL_GPUBuffer*>(i._buffer.id())};
            }

        SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(sdl3GC._command_buffer, storageTextureBindings, numReadWriteStorageTextures, storageBufferBindings, numReadWriteStorageBuffers);
        if(numReadOnlyStorageTextures > 0)
            SDL_BindGPUComputeStorageTextures(computePass, 0, readonlyStorageTextures, numReadOnlyStorageTextures);
        if(numReadOnlyStorageBuffers > 0)
            SDL_BindGPUComputeStorageBuffers(computePass, 0, readonlyStorageBuffers, numReadOnlyStorageBuffers);
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
