#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/inf/pipeline.h"

#include "bgfx/base/resource_base.h"

namespace ark::plugin::bgfx {

class DrawPipelineBgfx final : public ResourceBase<::bgfx::ProgramHandle, Pipeline> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_handle))
        {
        }
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

};

sp<Pipeline> PipelineFactoryBgfx::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
{
    std::map<PipelineInput::ShaderStage, String> shaders = bindings.layout()->getPreprocessedShaders(graphicsContext.renderContext());

}

}
