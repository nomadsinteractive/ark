#include "sdl3/impl/pipeline_factory/pipeline_factory_sdl3_gpu.h"

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "core/types/global.h"
#include "core/util/uploader_type.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_layout.h"
#include "renderer/inf/pipeline.h"
#include "renderer/util/render_util.h"

#include "sdl3/base/context_sdl3_gpu.h"
#include "sdl3/impl/texture/texture_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

namespace {

class SDL_ShaderCrossContext {
public:
    SDL_ShaderCrossContext()
    {
        SDL_ShaderCross_Init();
        _shader_format = SDL_ShaderCross_GetSPIRVShaderFormats();
    }
    ~SDL_ShaderCrossContext()
    {
        SDL_ShaderCross_Quit();
    }

    SDL_GPUShaderFormat _shader_format;
};

SDL_GPUShader* createGraphicsShader(SDL_GPUDevice* device, const ShaderLayout& pipelineInput, const StringView source, Enum::ShaderStageBit stageBit)
{
	const SDL_GPUShaderFormat backendFormats = Global<SDL_ShaderCrossContext>()->_shader_format;
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

    const Vector<uint32_t> binaries = RenderUtil::compileSPIR(source, stageBit, Ark::RENDERER_BACKEND_VULKAN);
    const void* bytecode = binaries.data();

    const Uint32 samplerCount = stageBit == Enum::SHADER_STAGE_BIT_FRAGMENT ? pipelineInput.samplerCount() : 0;

    Uint32 uniformBufferCount = 0;
    for(const ShaderLayout::UBO& i : pipelineInput.ubos())
        if(i._stages.has(stageBit))
            ++ uniformBufferCount;

    Uint32 storageBufferCount = 0;
    for(const ShaderLayout::SSBO& i : pipelineInput.ssbos())
        if(i._stages.has(stageBit))
            ++ storageBufferCount;

    Uint32 storageTextureCount = 0;

    const SDL_ShaderCross_SPIRV_Info spirvInfo = {
        static_cast<const Uint8*>(bytecode),
        binaries.size() * sizeof(uint32_t),
        entrypoint,
        stageBit == Enum::SHADER_STAGE_BIT_VERTEX ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT,
        false,
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

SDL_GPUVertexElementFormat toVertexElementFormat(const Attribute& i)
{
    switch(i.type())
    {
        case Attribute::TYPE_BYTE:
            if(i.length() == 2)
                return SDL_GPU_VERTEXELEMENTFORMAT_BYTE2;
            if(i.length() == 4)
                return SDL_GPU_VERTEXELEMENTFORMAT_BYTE4;
            break;
        case Attribute::TYPE_FLOAT: {
            constexpr SDL_GPUVertexElementFormat floatFormat[4] = {SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4};
            if(i.length() > 0 && i.length() < 5)
                return floatFormat[i.length()];
            break;
        }
        case Attribute::TYPE_INTEGER: {
            constexpr SDL_GPUVertexElementFormat intFormat[4] = {SDL_GPU_VERTEXELEMENTFORMAT_INT, SDL_GPU_VERTEXELEMENTFORMAT_INT2, SDL_GPU_VERTEXELEMENTFORMAT_INT3, SDL_GPU_VERTEXELEMENTFORMAT_INT4};
            if(i.length() > 0 && i.length() < 5)
                return intFormat[i.length()];
            break;
        }
        case Attribute::TYPE_SHORT:
            if(i.length() == 2)
                return SDL_GPU_VERTEXELEMENTFORMAT_SHORT2;
            if(i.length() == 4)
                return SDL_GPU_VERTEXELEMENTFORMAT_SHORT4;
            break;
        case Attribute::TYPE_UBYTE:
            if(i.length() == 2)
                return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2;
            if(i.length() == 4)
                return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4;
            break;
        case Attribute::TYPE_USHORT:
            if(i.length() == 2)
                return SDL_GPU_VERTEXELEMENTFORMAT_USHORT2;
            if(i.length() == 4)
                return SDL_GPU_VERTEXELEMENTFORMAT_USHORT4;
            break;
        default:
            break;
    }
    FATAL("Unsupported attribute type: %d length = %d", i.type(), i.length());
    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
}

SDL_GPUPrimitiveType toPrimitiveType(const Enum::RenderMode drawMode)
{
    switch(drawMode)
    {
        case Enum::RENDER_MODE_LINES:
            return SDL_GPU_PRIMITIVETYPE_LINELIST;
        case Enum::RENDER_MODE_POINTS:
            return SDL_GPU_PRIMITIVETYPE_POINTLIST;
        case Enum::RENDER_MODE_TRIANGLES:
            return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        case Enum::RENDER_MODE_TRIANGLE_STRIP:
            return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
        default:
            break;
    }
    FATAL("Unsupported render mode: %d", drawMode);
    return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
}

class DrawPipelineSDL3_GPU final : public Pipeline {
public:
    DrawPipelineSDL3_GPU(Enum::DrawProcedure drawProcedure, Enum::RenderMode drawMode, const sp<ShaderLayout>& pipelineInput, String vertexShader, String fragmentShader)
        : _draw_procedure(drawProcedure), _draw_mode(drawMode), _pipeline_input(pipelineInput), _vertex_shader(std::move(vertexShader)), _fragment_shader(std::move(fragmentShader)), _pipeline(nullptr) {
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
            SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
            SDL_GPUShader* vertexShader = createGraphicsShader(gpuDevice, _pipeline_input, _vertex_shader, Enum::SHADER_STAGE_BIT_VERTEX);
            SDL_GPUShader* fragmentShader = createGraphicsShader(gpuDevice, _pipeline_input, _fragment_shader, Enum::SHADER_STAGE_BIT_FRAGMENT);

            SDL_GPUVertexBufferDescription vertexBufferDescription[8];
            Uint32 numVertexBuffers = 0;

            SDL_GPUVertexAttribute vertexAttribute[32];
            Uint32 numVertexAttributes = 0;

            for(const auto& [k, v] : _pipeline_input->streamLayouts())
            {
                vertexBufferDescription[numVertexBuffers] = {
                    numVertexBuffers,
                    v.stride(),
                    k == 0 ? SDL_GPU_VERTEXINPUTRATE_VERTEX : SDL_GPU_VERTEXINPUTRATE_INSTANCE,
                    k
                };
                ++ numVertexBuffers;

                Uint32 location = 0;
                for(const Attribute& i : v.attributes().values())
                {
                    vertexAttribute[numVertexAttributes] = {
                        location ++,
                        i.divisor(),
                        toVertexElementFormat(i),
                        i.offset()
                    };
                    ++ numVertexAttributes;
                }
            }

            const SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
                vertexShader,
                fragmentShader,
                {
                    vertexBufferDescription,
                    numVertexBuffers,
                    vertexAttribute,
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
            };
            _pipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineCreateInfo);
        }
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DASSERT(drawingContext._vertices);
        DASSERT(drawingContext._indices);

        const PipelineBindings& pipelineBindings = drawingContext._bindings;
        const GraphicsContextSDL3_GPU& sdl3GC = graphicsContext.traits().ensure<GraphicsContextSDL3_GPU>();

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(sdl3GC._command_buffer, &sdl3GC._color_target_info, 1, nullptr);

        SDL_BindGPUGraphicsPipeline(renderPass, _pipeline);

        const SDL_GPUBufferBinding vertexBufferBinding = {reinterpret_cast<SDL_GPUBuffer*>(drawingContext._vertices.id()), 0};
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

        const SDL_GPUBufferBinding indexBufferBinding = {reinterpret_cast<SDL_GPUBuffer*>(drawingContext._indices.id()), 0};
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        SDL_GPUTextureSamplerBinding textureSamplerBinding[8];
        Uint32 samplerCount = 0;
        ASSERT(pipelineBindings.pipelineDescriptor()->samplers().size() < 8);
        for(const auto& [k, v] : pipelineBindings.pipelineDescriptor()->samplers())
        {
            TextureSDL3_GPU& texture = k->delegate().cast<TextureSDL3_GPU>();
            textureSamplerBinding[samplerCount ++] = {
                texture.texture(),
                texture.ensureSampler(ensureGPUDevice(graphicsContext))
            };
        }
        SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBinding, samplerCount);

        switch(_draw_procedure)
        {
            case Enum::DRAW_PROCEDURE_DRAW_ELEMENTS:
                SDL_DrawGPUIndexedPrimitives(renderPass, drawingContext._draw_count, 0, 0, 0, 0);
                break;
            case Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT: {
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
    Enum::DrawProcedure _draw_procedure;
    Enum::RenderMode _draw_mode;
    sp<ShaderLayout> _pipeline_input;
    String _vertex_shader;
    String _fragment_shader;

    SDL_GPUGraphicsPipeline* _pipeline;
};

class ComputePipelineSDL3_GPU final : public Pipeline {
public:
    ComputePipelineSDL3_GPU(String computeShader)
        : _compute_shader(std::move(computeShader)) {
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
            SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
            const char *entrypoint;

            if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
                format = SDL_GPU_SHADERFORMAT_SPIRV;
                entrypoint = "main";
            } else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
                format = SDL_GPU_SHADERFORMAT_MSL;
                entrypoint = "main0";
            } else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
                format = SDL_GPU_SHADERFORMAT_DXIL;
                entrypoint = "main";
            } else {
                SDL_Log("%s", "Unrecognized backend shader format!");
                return;
            }

            const Vector<uint32_t> binaries = RenderUtil::compileSPIR(_compute_shader, Enum::SHADER_STAGE_BIT_COMPUTE, Ark::RENDERER_BACKEND_VULKAN);
            const void* bytecode = binaries.data();

            const SDL_GPUComputePipelineCreateInfo computePipelineCreateInfo = {
                binaries.size(),
                static_cast<const Uint8*>(bytecode),
                entrypoint,
                format
            };

            _pipeline = SDL_CreateGPUComputePipeline(gpuDevice, &computePipelineCreateInfo);
            if(!_pipeline)
            {
                SDL_Log("Failed to create compute pipeline!");
            }
        }
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DFATAL("Shouldn't be here");
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        SDL_GPUStorageTextureReadWriteBinding storageTextureBindings[8];
        SDL_GPUStorageBufferReadWriteBinding storageBufferBindings[8];

        const GraphicsContextSDL3_GPU& sdl3GC = graphicsContext.traits().ensure<GraphicsContextSDL3_GPU>();

        SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(
            sdl3GC._command_buffer,
            storageTextureBindings,
            1,
            storageBufferBindings,
            0
        );

        SDL_BindGPUComputePipeline(computePass, _pipeline);
        SDL_DispatchGPUCompute(computePass, computeContext._num_work_groups[0], computeContext._num_work_groups[1], computeContext._num_work_groups[2]);
        SDL_EndGPUComputePass(computePass);
    }

private:
    String _compute_shader;

    SDL_GPUComputePipeline* _pipeline;
};

}

sp<Pipeline> PipelineFactorySDL3_GPU::buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor, std::map<Enum::ShaderStageBit, String> stages)
{
    if(const auto vIter = stages.find(Enum::SHADER_STAGE_BIT_VERTEX); vIter != stages.end())
    {
        const Enum::DrawProcedure drawProcedure = pipelineDescriptor.drawProcedure();
        const auto fIter = stages.find(Enum::SHADER_STAGE_BIT_FRAGMENT);
        CHECK(fIter != stages.end(), "Pipeline has no fragment shader(only vertex shader available)");
        return sp<Pipeline>::make<DrawPipelineSDL3_GPU>(drawProcedure, pipelineDescriptor.mode(), pipelineDescriptor.input(), std::move(vIter->second), std::move(fIter->second));
    }
    const auto cIter = stages.find(Enum::SHADER_STAGE_BIT_COMPUTE);
    CHECK(cIter != stages.end(), "Pipeline has no compute shader");
    return sp<Pipeline>::make<ComputePipelineSDL3_GPU>(std::move(cIter->second));
}

}
