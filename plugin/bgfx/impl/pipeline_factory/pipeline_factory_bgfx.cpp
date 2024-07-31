#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/inf/pipeline.h"

#include "bgfx/base/bgfx_context.h"
#include "bgfx/base/resource_base.h"

namespace ark::plugin::bgfx {

namespace {

class DrawPipelineBgfx final : public ResourceBase<::bgfx::ProgramHandle, Pipeline> {
public:
    DrawPipelineBgfx(String vertexShader, String fragmentShader)
        : _vertex_shader(std::move(vertexShader)), _fragment_shader(std::move(fragmentShader)) {
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_handle))
        {
            const auto vHandle = ::bgfx::createShader(::bgfx::makeRef(_vertex_shader.c_str(), _vertex_shader.size()));
            const auto fHandle = ::bgfx::createShader(::bgfx::makeRef(_fragment_shader.c_str(), _fragment_shader.size()));
            _handle = ::bgfx::createProgram(vHandle, fHandle, true);
        }
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        const BgfxContext& ctx = graphicsContext.attachments().ensure<BgfxContext>();
        ::bgfx::submit(ctx._view_id, _handle);
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        DFATAL("Shouldn't be here");
    }

private:
    String _vertex_shader;
    String _fragment_shader;
};

class ComputePipelineBgfx final : public ResourceBase<::bgfx::ProgramHandle, Pipeline> {
public:
    ComputePipelineBgfx(String computeShader)
        : _compute_shader(std::move(computeShader)) {
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_handle))
        {
            const auto cHandle = ::bgfx::createShader(::bgfx::makeRef(_compute_shader.c_str(), _compute_shader.size()));
            _handle = ::bgfx::createProgram(cHandle, true);
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

sp<Pipeline> PipelineFactoryBgfx::buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& bindings)
{
    std::map<PipelineInput::ShaderStage, String> shaders = bindings.layout()->getPreprocessedShaders(graphicsContext.renderContext());
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
