#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"

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

::bgfx::ShaderHandle createShader(const String& source, PipelineInput::ShaderStage stage)
{
    const std::vector<uint32_t> binaries = RenderUtil::compileSPIR(source, stage, Ark::RENDERER_TARGET_OPENGL);
    return ::bgfx::createShader(::bgfx::copy(binaries.data(), binaries.size() * sizeof(uint32_t)));
}

class DrawPipelineBgfx final : public ResourceBase<::bgfx::ProgramHandle, Pipeline> {
public:
    DrawPipelineBgfx(String vertexShader, String fragmentShader)
        : _vertex_shader(std::move(vertexShader)), _fragment_shader(std::move(fragmentShader)) {
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
        {
            const auto vHandle = createShader(_vertex_shader, PipelineInput::SHADER_STAGE_VERTEX);
            const auto fHandle = createShader(_fragment_shader.c_str(), PipelineInput::SHADER_STAGE_FRAGMENT);
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
        return sp<Pipeline>::make<DrawPipelineBgfx>(std::move(vIter->second), std::move(fIter->second));
    }
    const auto cIter = shaders.find(PipelineInput::SHADER_STAGE_COMPUTE);
    CHECK(cIter != shaders.end(), "Pipeline has no compute shader");
    return sp<Pipeline>::make<ComputePipelineBgfx>(std::move(cIter->second));
}

}
