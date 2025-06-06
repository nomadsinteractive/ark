#include "renderer/impl/draw_decorator/draw_decorator_factory_compute.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/inf/draw_decorator.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class DrawEventsPreDrawCompute final : public DrawDecorator {
public:
    DrawEventsPreDrawCompute(ComputeContext computeContext)
        : _compute_context(std::move(computeContext))
    {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        _compute_context._bindings = context._bindings;
        context._bindings->ensureComputePipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

private:
    ComputeContext _compute_context;
};

class DrawEventsPostDrawCompute final : public DrawDecorator {
public:
    DrawEventsPostDrawCompute(ComputeContext computeContext)
        : _compute_context(std::move(computeContext))
    {
    }

    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        _compute_context._bindings = context._bindings;
        context._bindings->ensureComputePipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

private:
    ComputeContext _compute_context;
};

}

DrawDecoratorFactoryCompute::DrawDecoratorFactoryCompute(sp<PipelineLayout> pipelineLayout, const V3i numWorkGroups, const bool atPostDraw)
    : _pipeline_layout(std::move(pipelineLayout)), _num_work_groups(numWorkGroups), _at_post_draw(atPostDraw)
{
}

sp<DrawDecorator> DrawDecoratorFactoryCompute::makeDrawDecorator(const RenderRequest& renderRequest)
{
    ComputeContext computeCtx(nullptr, _pipeline_layout->takeBufferSnapshot(renderRequest, true), _num_work_groups);
    return _at_post_draw ? sp<DrawDecorator>::make<DrawEventsPostDrawCompute>(std::move(computeCtx)) : sp<DrawDecorator>::make<DrawEventsPreDrawCompute>(std::move(computeCtx));
}

}
