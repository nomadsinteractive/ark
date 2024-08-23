#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"

#include <bx/hash.h>
#include <bx/readerwriter.h>

#include "core/util/uploader_type.h"

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
#include "bgfx/impl/buffer/indirect_buffer_bgfx.h"
#include "bgfx/impl/texture/texture_bgfx.h"
#include "core/impl/writable/writable_memory.h"

namespace ark::plugin::bgfx {

namespace {

char toBgfxShaderTypeMagic(Enum::ShaderStageBit stage)
{
    switch(stage)
    {
        case Enum::SHADER_STAGE_BIT_VERTEX:
            return 'V';
        case Enum::SHADER_STAGE_BIT_FRAGMENT:
            return 'F';
        case Enum::SHADER_STAGE_BIT_COMPUTE:
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

uint16_t toBgfxAttribId(Attribute::Usage usage, uint32_t& customId)
{
/*
static AttribToId s_attribToId[] =
{
    // NOTICE:
    // Attrib must be in order how it appears in Attrib::Enum! id is
    // unique and should not be changed if new Attribs are added.
    { Attrib::Position,  0x0001 },
    { Attrib::Normal,    0x0002 },
    { Attrib::Tangent,   0x0003 },
    { Attrib::Bitangent, 0x0004 },
    { Attrib::Color0,    0x0005 },
    { Attrib::Color1,    0x0006 },
    { Attrib::Color2,    0x0018 },
    { Attrib::Color3,    0x0019 },
    { Attrib::Indices,   0x000e },
    { Attrib::Weight,    0x000f },
    { Attrib::TexCoord0, 0x0010 },
    { Attrib::TexCoord1, 0x0011 },
    { Attrib::TexCoord2, 0x0012 },
    { Attrib::TexCoord3, 0x0013 },
    { Attrib::TexCoord4, 0x0014 },
    { Attrib::TexCoord5, 0x0015 },
    { Attrib::TexCoord6, 0x0016 },
    { Attrib::TexCoord7, 0x0017 },
};
*/
    switch(usage)
    {
        case Attribute::USAGE_CUSTOM:
            if(customId < 3)
            {
                constexpr uint16_t cs[3] = {0x0006, 0x0018, 0x0019};
                return cs[customId ++];
            }
            if(customId < 10)
                return 0x11 + (customId ++) - 3;
            FATAL("Too many custom attributes");
        case Attribute::USAGE_POSITION:
            return 1;
        case Attribute::USAGE_TEX_COORD:
            return 0x10;
        case Attribute::USAGE_COLOR:
            return 0x05;
        case Attribute::USAGE_NORMAL:
            return 0x02;
        case Attribute::USAGE_TANGENT:
            return 0x03;
        case Attribute::USAGE_BITANGENT:
            return 0x04;
        default:
            FATAL("Unsupported uniform type: %d", usage);
            break;
    }
    return 0;
}

uint8_t toBgfxTextureDimension(Texture::Type type)
{
// { TextureDimension::Dimension1D,        0x01 },
// { TextureDimension::Dimension2D,        0x02 },
// { TextureDimension::Dimension2DArray,   0x03 },
// { TextureDimension::DimensionCube,      0x04 },
// { TextureDimension::DimensionCubeArray, 0x05 },
// { TextureDimension::Dimension3D,        0x06 },
    switch(type)
    {
        case Texture::TYPE_2D:
            return 0x02;
        case Texture::TYPE_CUBEMAP:
            return 0x04;
        default:
            FATAL("Unknown texture type: %d", type);
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
    uint8_t  texComponent = 0;
    uint8_t  texDimension = 0;
    uint16_t texFormat = 0;
};

struct alignas(1) BgfxShaderAttributeChunk {
    uint16_t id;
};

#pragma pack(pop)

::bgfx::ShaderHandle createShader(const PipelineInput& pipelineInput, const String& source, Enum::ShaderStageBit stage)
{
    const char bgfxChunkMagic[4] = {toBgfxShaderTypeMagic(stage), 'S', 'H', 11};
    const std::vector<uint32_t> binaries = RenderUtil::compileSPIR(source, stage, Ark::RENDERER_TARGET_VULKAN);
    const void* bytecode = binaries.data();
    const uint32_t bytecodeSize = binaries.size() * sizeof(uint32_t);
    // const void* bytecode = source.c_str();
    // const uint32_t bytecodeSize = source.size();
    BgfxShaderHeader shaderHeader;
    shaderHeader.magic = *reinterpret_cast<const uint32_t*>(bgfxChunkMagic);
    std::vector<std::pair<String, BgfxShaderUniformChunk>> uniformChunks;
    uint32_t uboSize = 0;
    uint32_t ssboSize = 0;
    uint32_t dynamicDataSize = 0;
    for(const PipelineInput::UBO& i : pipelineInput.ubos())
        if(i.stages().has(stage))
            for(const auto& [name, uniform] : i.uniforms())
            {
                String tname = translatePredefinedName(name);
                uboSize += uniform->size();
                dynamicDataSize += (tname.size() + 1);

                BgfxShaderUniformChunk uniformChunk;
                uniformChunk.type = toBgfxUniformType(uniform->type());
                uniformChunk.num = 1;
                uniformChunk.regIndex = i.binding();
                uniformChunk.regCount = (uniform->size() + 15) >> 4;
                uniformChunks.emplace_back(std::make_pair(std::move(tname), uniformChunk));
            }

    uint32_t binding = 2;
    if(stage == Enum::SHADER_STAGE_BIT_FRAGMENT)
        for(const String& i : pipelineInput.samplerNames())
        {
            dynamicDataSize += (i.size() + 1);

            BgfxShaderUniformChunk uniformChunk;
            uniformChunk.type = ::bgfx::UniformType::Sampler;
            uniformChunk.num = 1;
            uniformChunk.regIndex = binding++;
            uniformChunk.regCount = 1;
            uniformChunk.texDimension = toBgfxTextureDimension(Texture::TYPE_2D);
            uniformChunks.emplace_back(std::make_pair(i, uniformChunk));
        }

    for(const PipelineInput::SSBO& i : pipelineInput.ssbos())
        ssboSize += i._buffer.size();

    uint32_t customId = 0;
    std::vector<BgfxShaderAttributeChunk> attributeChunks;
    if(stage == Enum::SHADER_STAGE_BIT_VERTEX)
        for(const auto& [name, attribute] : pipelineInput.getStreamLayout(0).attributes())
            attributeChunks.push_back({toBgfxAttribId(attribute.usage(), customId)});

    shaderHeader.count = uniformChunks.size();
    bx::Error err;
    const ::bgfx::Memory* mem = ::bgfx::alloc(sizeof(BgfxShaderHeader) + bytecodeSize + 4 + 2 + dynamicDataSize + uniformChunks.size() * sizeof(BgfxShaderUniformChunk) + attributeChunks.size() * sizeof(BgfxShaderAttributeChunk) + 2);
    bx::StaticMemoryBlockWriter writer(mem->data, mem->size);
    writer.write(&shaderHeader, sizeof(shaderHeader), &err);
    for(const auto& [name, chunk] : uniformChunks)
    {
        uint8_t nameSize = name.size();
        bx::write(&writer, nameSize, &err);
        writer.write(name.c_str(), nameSize, &err);
        bx::write(&writer, chunk, &err);
    }
    bx::write(&writer, bytecodeSize, &err);
    writer.write(bytecode, bytecodeSize, &err);
    bx::write(&writer, static_cast<uint8_t>(0), &err);
    bx::write(&writer, static_cast<uint8_t>(attributeChunks.size()), &err);
    for(const auto& i: attributeChunks)
        bx::write(&writer, i.id, &err);
    bx::write(&writer, static_cast<uint16_t>(uboSize + ssboSize), &err);
    const ::bgfx::ShaderHandle handle = ::bgfx::createShader(mem);
    ASSERT(::bgfx::isValid(handle));
    return handle;
}

struct DrawPipelineBgfx final : ResourceBase<::bgfx::ProgramHandle, Pipeline> {
    DrawPipelineBgfx(Enum::DrawProcedure drawProcedure, Enum::RenderMode drawMode, const sp<PipelineInput>& pipelineInput, String vertexShader, String fragmentShader)
        : _draw_procedure(drawProcedure), _draw_mode(drawMode), _pipeline_input(pipelineInput), _vertex_shader(std::move(vertexShader)), _fragment_shader(std::move(fragmentShader)) {
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
        {
            const auto vHandle = createShader(_pipeline_input, _vertex_shader, Enum::SHADER_STAGE_BIT_VERTEX);
            const auto fHandle = createShader(_pipeline_input, _fragment_shader, Enum::SHADER_STAGE_BIT_FRAGMENT);
            _handle.reset(::bgfx::createProgram(vHandle, fHandle, true));
        }
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        if(_sampler_slots.empty() && !drawingContext._bindings->pipelineInput()->samplerNames().empty())
        {
            uint8_t textureUint = 0;
            for(size_t i = 0; i < drawingContext._bindings->pipelineInput()->samplerNames().size(); ++i)
            {
                const String& name = drawingContext._bindings->pipelineInput()->samplerNames().at(i);
                const auto& [texture, bindingSet] = drawingContext._bindings->pipelineDescriptor()->samplers().at(i);
                const sp<TextureBgfx> textureBgfx = texture->delegate().cast<TextureBgfx>();
                _sampler_slots.push_back({::bgfx::createUniform(name.c_str(), ::bgfx::UniformType::Sampler), std::move(textureBgfx), textureUint++});
            }
        }
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        DASSERT(drawingContext._vertices);
        DASSERT(drawingContext._indices);

        const BgfxContext& ctx = graphicsContext.attachments().ensure<BgfxContext>();

        for(const auto& [uniform, texture, stage] : _sampler_slots)
            ::bgfx::setTexture(stage, uniform, texture->handle());

        {
            const Camera& camera = drawingContext._bindings->pipelineInput()->camera();
            const M4 view = camera.view()->val();
            const M4 proj = camera.projection()->val();
            ::bgfx::setViewTransform(ctx._view_id, &view, &proj);
        }

        {
            uint64_t state = BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO);
            if(_draw_mode == Enum::RENDER_MODE_TRIANGLE_STRIP)
                state |= BGFX_STATE_PT_TRISTRIP;
            ::bgfx::setState(state);
        }

        const sp<BufferBase> vertices = drawingContext._vertices.delegate().cast<BufferBase>();
        const sp<BufferBase> indices = drawingContext._indices.delegate().cast<BufferBase>();
        DASSERT(vertices->type() == Buffer::TYPE_VERTEX);
        DASSERT(indices->type() == Buffer::TYPE_INDEX);

        switch(_draw_procedure)
        {
            case Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT: {
                const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();
                const sp<IndirectBufferBgfx> indirectBuffer = param._indirect_cmds.delegate().cast<IndirectBufferBgfx>();
                if(param._indirect_cmds._uploader)
                    indirectCommands = UploaderType::toBytes(param._indirect_cmds._uploader);

                CHECK(param._divided_buffer_snapshots.size() < 2, "Only one stream of instance buffer allowed");

                uint32_t instanceCount = 0;
                const DrawingParams::DrawElementsIndirectCommand* ic = reinterpret_cast<const DrawingParams::DrawElementsIndirectCommand*>(indirectCommands.data());
                for(size_t i = 0; i < param._indirect_cmd_count; ++i)
                    instanceCount += ic[i]._instance_count;

                ::bgfx::InstanceDataBuffer idb;
                for(const auto& [divisor, buffer] : param._divided_buffer_snapshots)
                {
                    const PipelineInput::StreamLayout& sl = drawingContext._bindings->pipelineInput()->getStreamLayout(divisor);
                    const uint32_t availInstanceCount = ::bgfx::getAvailInstanceDataBuffer(instanceCount, sl.stride());
                    ::bgfx::allocInstanceDataBuffer(&idb, availInstanceCount, sl.stride());
                    if(buffer._uploader)
                        instanceDataBuffers = UploaderType::toBytes(buffer._uploader);
                     memcpy(idb.data, instanceDataBuffers.data(), instanceDataBuffers.size());
                }

                ::bgfx::setInstanceDataBuffer(&idb);
                for(size_t i = 0; i < param._indirect_cmd_count; ++i)
                {
                    vertices->bindRange(ic[i]._base_vertex, UINT32_MAX);
                    indices->bindRange(ic[i]._first_index, ic[i]._count);
                    ::bgfx::submit(ctx._view_id, _handle);
                }
                break;
            }
            default: {
                vertices->bind();
                indices->bindRange(0, drawingContext._draw_count);
                ::bgfx::submit(ctx._view_id, _handle);
                break;
            }
        }
    }
//TODO: Move them to somewhere else
    std::vector<uint8_t> indirectCommands;
    std::vector<uint8_t> instanceDataBuffers;
    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        DFATAL("Shouldn't be here");
    }

    struct SamplerSlot {
        Handle<::bgfx::UniformHandle> _uniform;
        sp<TextureBgfx> _texture;
        uint8_t _stage;
    };

    Enum::DrawProcedure _draw_procedure;
    Enum::RenderMode _draw_mode;
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
    std::map<Enum::ShaderStageBit, String> shaders = bindings.pipelineLayout()->getPreprocessedShaders(graphicsContext.renderContext());
    if(const auto vIter = shaders.find(Enum::SHADER_STAGE_BIT_VERTEX); vIter != shaders.end())
    {
        const PipelineDescriptor& pipelineDescriptor = bindings.pipelineDescriptor();
        const Enum::DrawProcedure drawProcedure = pipelineDescriptor.drawProcedure();
        const auto fIter = shaders.find(Enum::SHADER_STAGE_BIT_FRAGMENT);
        CHECK(fIter != shaders.end(), "Pipeline has no fragment shader(only vertex shader available)");
        return sp<Pipeline>::make<DrawPipelineBgfx>(drawProcedure, pipelineDescriptor.mode(), bindings.pipelineInput(), std::move(vIter->second), std::move(fIter->second));
    }
    const auto cIter = shaders.find(Enum::SHADER_STAGE_BIT_COMPUTE);
    CHECK(cIter != shaders.end(), "Pipeline has no compute shader");
    return sp<Pipeline>::make<ComputePipelineBgfx>(std::move(cIter->second));
}

}
