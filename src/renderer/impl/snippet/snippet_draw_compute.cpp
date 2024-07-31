#include "renderer/impl/snippet/snippet_draw_compute.h"

#include "core/util/bean_utils.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class DrawEventsPreDrawCompute final : public Snippet::DrawEvents {
public:
    DrawEventsPreDrawCompute(ComputeContext computeContext)
        : _compute_context(std::move(computeContext))
    {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& /*context*/) override
    {
        _compute_context._pipeline_context._bindings->getPipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

    void postDraw(GraphicsContext& graphicsContext) override {}

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

    void postDraw(GraphicsContext& graphicsContext) override
    {
        _compute_context._pipeline_context._bindings->getPipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

private:
    ComputeContext _compute_context;
};

}

SnippetDrawCompute::SnippetDrawCompute(sp<Shader> shader, std::array<sp<Integer>, 3> numWorkGroups, bool atPostDraw)
    : _shader(std::move(shader)), _num_work_groups(std::move(numWorkGroups)), _shader_bindings(_shader->makeBindings(Buffer(), Enum::RENDER_MODE_NONE, Enum::DRAW_PROCEDURE_DRAW_ARRAYS)),
      _at_post_draw(atPostDraw)
{
}

sp<Snippet::DrawEvents> SnippetDrawCompute::makeDrawEvents(const RenderRequest& renderRequest)
{
    std::array<int32_t, 3> numWorkGroups = {
        _num_work_groups.at(0) ? _num_work_groups.at(0)->val() : 1,
        _num_work_groups.at(1) ? _num_work_groups.at(1)->val() : 1,
        _num_work_groups.at(2) ? _num_work_groups.at(2)->val() : 1
    };
    ComputeContext computeCtx({_shader_bindings, _shader->takeUBOSnapshot(renderRequest), _shader->takeSSBOSnapshot(renderRequest)}, std::move(numWorkGroups));
    return _at_post_draw ? sp<DrawEvents>::make<DrawEventsPostDrawCompute>(std::move(computeCtx)) : sp<DrawEvents>::make<DrawEventsPreDrawCompute>(std::move(computeCtx));
}

SnippetDrawCompute::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER)),
    //TODO: better configuration ideas?
    _at_post_draw(Documents::getAttribute(manifest, "at") == "post_draw")
{
    BeanUtils::split(factory, manifest, "num-work-groups", _num_work_groups[0], _num_work_groups[1], _num_work_groups[2]);
}

sp<Snippet> SnippetDrawCompute::BUILDER::build(const Scope& args)
{
    std::array<sp<Integer>, 3> numWorkGroups = {
        _num_work_groups.at(0) ? _num_work_groups.at(0)->build(args) : nullptr,
        _num_work_groups.at(1) ? _num_work_groups.at(1)->build(args) : nullptr,
        _num_work_groups.at(2) ? _num_work_groups.at(2)->build(args) : nullptr,
    };
    return sp<SnippetDrawCompute>::make(_shader->build(args), std::move(numWorkGroups), _at_post_draw);
}

}
