#include "renderer/impl/snippet/snippet_draw_compute.h"

#include "core/util/bean_utils.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class DrawEventsCompute : public Snippet::DrawEvents {
public:
    DrawEventsCompute(sp<Shader> shader, ComputeContext computeContext)
        : _shader(std::move(shader)), _compute_context(std::move(computeContext)) {
    }

    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& /*context*/) override {
        _compute_context._shader_bindings->getPipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

    virtual void postDraw(GraphicsContext& graphicsContext) override {
    }

private:
    sp<Shader> _shader;

    ComputeContext _compute_context;
};

}

SnippetDrawCompute::SnippetDrawCompute(sp<Shader> shader, std::array<sp<Integer>, 3> numWorkGroups)
    : _shader(std::move(shader)), _num_work_groups(std::move(numWorkGroups)), _shader_bindings(_shader->makeBindings(Buffer(), Enum::RENDER_MODE_NONE, Enum::DRAW_PROCEDURE_DRAW_ARRAYS))
{
}

sp<Snippet::DrawEvents> SnippetDrawCompute::makeDrawEvents(const RenderRequest& renderRequest)
{
    std::array<int32_t, 3> numWorkGroups = {
        _num_work_groups.at(0) ? _num_work_groups.at(0)->val() : 1,
        _num_work_groups.at(1) ? _num_work_groups.at(1)->val() : 1,
        _num_work_groups.at(2) ? _num_work_groups.at(2)->val() : 1
    };
    return sp<DrawEventsCompute>::make(_shader, ComputeContext(_shader_bindings, _shader->takeUBOSnapshot(renderRequest), _shader->takeSSBOSnapshot(renderRequest), std::move(numWorkGroups)));
}

SnippetDrawCompute::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER))
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
    return sp<SnippetDrawCompute>::make(_shader->build(args), std::move(numWorkGroups));
}

}
