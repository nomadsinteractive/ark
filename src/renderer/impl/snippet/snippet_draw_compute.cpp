#include "renderer/impl/snippet/snippet_draw_compute.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class DrawEventsPreDrawCompute final : public Snippet::DrawEvents {
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

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {}

private:
    ComputeContext _compute_context;
};

class DrawEventsPostDrawCompute final : public Snippet::DrawEvents {
public:
    DrawEventsPostDrawCompute(ComputeContext computeContext)
        : _compute_context(std::move(computeContext))
    {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& /*context*/) override {}

    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        _compute_context._bindings = context._bindings;
        context._bindings->ensureComputePipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

private:
    ComputeContext _compute_context;
};

}

SnippetDrawCompute::SnippetDrawCompute(sp<PipelineInput> pipelineInput, std::array<uint32_t, 3> numWorkGroups, bool atPostDraw)
    : _pipeline_input(std::move(pipelineInput)), _num_work_groups(numWorkGroups), _at_post_draw(atPostDraw)
{
}

sp<Snippet::DrawEvents> SnippetDrawCompute::makeDrawEvents(const RenderRequest& renderRequest)
{
    ComputeContext computeCtx(nullptr, _pipeline_input->takeBufferSnapshot(renderRequest, true), _num_work_groups);
    return _at_post_draw ? sp<DrawEvents>::make<DrawEventsPostDrawCompute>(std::move(computeCtx)) : sp<DrawEvents>::make<DrawEventsPreDrawCompute>(std::move(computeCtx));
}

}
