#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"

#include <bx/hash.h>
#include <bx/readerwriter.h>

#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/inf/pipeline.h"
#include "renderer/util/render_util.h"

#include "bgfx/base/bgfx_context.h"
#include "bgfx/base/handle.h"
#include "bgfx/base/resource_base.h"
#include "bgfx/impl/buffer/buffer_base.h"
#include "bgfx/impl/texture/texture_bgfx.h"

namespace ark::plugin::bgfx {

namespace {

char toBgfxShaderTypeMagic(PipelineInput::ShaderStage stage)
{
    switch(stage)
    {
        case PipelineInput::SHADER_STAGE_VERTEX:
            return 'V';
        case PipelineInput::SHADER_STAGE_FRAGMENT:
            return 'F';
        case PipelineInput::SHADER_STAGE_COMPUTE:
            return 'C';
        default:
            FATAL("Unsupported shader type: %d", stage);
            break;
    }
    return 0;
}

uint8_t toBgfxUniformType(Uniform::Type type)
{
    switch(type)
    {
        case Uniform::TYPE_MAT3:
            return ::bgfx::UniformType::Mat3;
        case Uniform::TYPE_MAT4:
            return ::bgfx::UniformType::Mat4;
        case Uniform::TYPE_F2:
        case Uniform::TYPE_F4:
        case Uniform::TYPE_F4V:
            return ::bgfx::UniformType::Vec4;
        case Uniform::TYPE_SAMPLER2D:
            return ::bgfx::UniformType::Sampler;
        default:
            FATAL("Unsupported uniform type: %d", type);
        break;
    }
    return 0;
}

String translatePredefinedName(const String& name)
{
//TODO: Maybe we can figure out the translation by the macros?
    if(name == "u_VP")
        return "u_viewProj";
    if(name == "u_View")
        return "u_view";
    if(name == "u_Projection")
        return "u_proj";
    return name;
}

#pragma pack(1)

struct alignas(1) BgfxShaderHeader {
    uint32_t magic;
//TODO: Honor those hash values
    uint32_t hashIn = 0;
    uint32_t hashOut = 0;
    uint16_t count;
};

struct alignas(1) BgfxShaderUniformChunk {
    uint8_t type;
    uint8_t num;
    uint16_t regIndex;
    uint16_t regCount;
    uint16_t texInfo;
    uint16_t texFormat = 0;
};

struct alignas(1) BgfxShaderAttributeChunk {
    uint16_t id;
};

#pragma pack(pop)

::bgfx::ShaderHandle createShader(const PipelineInput& pipelineInput, const String& source, PipelineInput::ShaderStage stage)
{
    const char bgfxChunkMagic[4] = {toBgfxShaderTypeMagic(stage), 'S', 'H', 11};
    const std::vector<uint32_t> binaries = RenderUtil::compileSPIR(source, stage, Ark::RENDERER_TARGET_VULKAN);
    const void* bytecode = binaries.data();
    const uint32_t bytecodeSize = binaries.size() * sizeof(uint32_t);
    // const void* bytecode = source.c_str();
    // const uint32_t bytecodeSize = source.size();
    const ::bgfx::Memory* mem = ::bgfx::alloc(sizeof(BgfxShaderHeader) + bytecodeSize + 32);
    BgfxShaderHeader shaderHeader;
    shaderHeader.magic = *reinterpret_cast<const uint32_t*>(bgfxChunkMagic);
    std::vector<std::pair<String, BgfxShaderUniformChunk>> uniformChunks;
    for(const PipelineInput::UBO& i : pipelineInput.ubos())
    {
        for(const auto& [name, uniform] : i.uniforms())
        {
            BgfxShaderUniformChunk uniformChunk;
            uniformChunk.type = toBgfxUniformType(uniform->type());
            uniformChunk.num = 1;
            uniformChunk.regIndex = uniform->binding();
            uniformChunk.regCount = uniform->length();
            uniformChunk.texInfo = 0;
            uniformChunks.emplace_back(std::make_pair(translatePredefinedName(name), uniformChunk));
        }
    }

    shaderHeader.count = uniformChunks.size();
    bx::Error err;
    bx::StaticMemoryBlockWriter writer(mem->data, mem->size);
    writer.write(&shaderHeader, sizeof(shaderHeader), &err);
    for(const auto& [name, chunk] : uniformChunks)
    {
        uint8_t nameSize = name.size();
        writer.write(&nameSize, sizeof(nameSize), &err);
        writer.write(name.c_str(), nameSize, &err);
        writer.write(&chunk, sizeof(chunk), &err);
    }
    writer.write(&bytecodeSize, sizeof(bytecodeSize), &err);
    writer.write(bytecode, bytecodeSize, &err);
    uint8_t nul = 0;
    writer.write(&nul, sizeof(nul), &err);
    const ::bgfx::ShaderHandle handle = ::bgfx::createShader(mem);
    ASSERT(::bgfx::isValid(handle));
    return handle;
}

class DrawPipelineBgfx final : public ResourceBase<::bgfx::ProgramHandle, Pipeline> {
public:
    DrawPipelineBgfx(const sp<PipelineInput>& pipelineInput, String vertexShader, String fragmentShader)
        : _pipeline_input(pipelineInput), _vertex_shader(std::move(vertexShader)), _fragment_shader(std::move(fragmentShader)) {
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
        {
            const auto vHandle = createShader(_pipeline_input, _vertex_shader, PipelineInput::SHADER_STAGE_VERTEX);
            const auto fHandle = createShader(_pipeline_input, _fragment_shader, PipelineInput::SHADER_STAGE_FRAGMENT);
            _handle.reset(::bgfx::createProgram(vHandle, fHandle, true));
        }
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        if(_sampler_slots.empty() && drawingContext._pipeline_snapshot._bindings->samplers().size())
        {
            uint8_t textureUint = 0;
            for(const auto& [name, texture] : drawingContext._pipeline_snapshot._bindings->samplers())
            {
                const sp<TextureBgfx> textureBgfx = texture->delegate().cast<TextureBgfx>();
                _sampler_slots.push_back({::bgfx::createUniform(name.c_str(), ::bgfx::UniformType::Sampler), std::move(textureBgfx), textureUint++});
            }
        }
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DASSERT(drawingContext._vertices);
        DASSERT(drawingContext._indices);

        const sp<BufferBase> vertices = drawingContext._vertices.delegate().cast<BufferBase>();
        DASSERT(vertices->type() == Buffer::TYPE_VERTEX);
        vertices->bind();

        const sp<BufferBase> indices = drawingContext._indices.delegate().cast<BufferBase>();
        DASSERT(indices->type() == Buffer::TYPE_INDEX);
        indices->bind();

        for(const auto& [uniform, texture, stage] : _sampler_slots)
            ::bgfx::setTexture(stage, uniform, texture->handle());

        const Camera& camera = drawingContext._pipeline_snapshot._bindings->pipelineInput()->camera();
        const BgfxContext& ctx = graphicsContext.attachments().ensure<BgfxContext>();
        const M4 view = camera.view()->val();
        const M4 proj = camera.projection()->val();
        ::bgfx::setViewTransform(ctx._view_id, &view, &proj);
        ::bgfx::submit(ctx._view_id, _handle);
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        DFATAL("Shouldn't be here");
    }

private:
    struct SamplerSlot {
        Handle<::bgfx::UniformHandle> _uniform;
        sp<TextureBgfx> _texture;
        uint8_t _stage;
    };

private:
    sp<PipelineInput> _pipeline_input;
    String _vertex_shader;
    String _fragment_shader;

    std::vector<SamplerSlot> _sampler_slots;
};

class ComputePipelineBgfx final : public ResourceBase<::bgfx::ProgramHandle, Pipeline> {
public:
    ComputePipelineBgfx(String computeShader)
        : _compute_shader(std::move(computeShader)) {
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
        {
            const auto cHandle = ::bgfx::createShader(::bgfx::makeRef(_compute_shader.c_str(), _compute_shader.size()));
            _handle.reset(::bgfx::createProgram(cHandle, true));
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
        const BgfxContext& ctx = graphicsContext.attachments().ensure<BgfxContext>();
        ::bgfx::dispatch(ctx._view_id, _handle);
    }

private:
    String _compute_shader;
};

}

sp<Pipeline> PipelineFactoryBgfx::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
{
    std::map<PipelineInput::ShaderStage, String> shaders = bindings.pipelineLayout()->getPreprocessedShaders(graphicsContext.renderContext());
    if(const auto vIter = shaders.find(PipelineInput::SHADER_STAGE_VERTEX); vIter != shaders.end())
    {
        const auto fIter = shaders.find(PipelineInput::SHADER_STAGE_FRAGMENT);
        CHECK(fIter != shaders.end(), "Pipeline has no fragment shader(only vertex shader available)");
        return sp<Pipeline>::make<DrawPipelineBgfx>(bindings.pipelineInput(), std::move(vIter->second), std::move(fIter->second));
    }
    const auto cIter = shaders.find(PipelineInput::SHADER_STAGE_COMPUTE);
    CHECK(cIter != shaders.end(), "Pipeline has no compute shader");
    return sp<Pipeline>::make<ComputePipelineBgfx>(std::move(cIter->second));
}

}
