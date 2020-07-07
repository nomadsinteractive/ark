#include "renderer/impl/snippet/snippet_draw_compute.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class DrawEventsCompute : public Snippet::DrawEvents {
public:
    DrawEventsCompute(const sp<Shader>& shader, const sp<Buffer>& buffer, ComputeContext computeContext)
        : _shader(shader), _buffer(buffer), _compute_context(std::move(computeContext)) {
    }

    virtual void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
    }

    virtual void postDraw(GraphicsContext& graphicsContext) override {
        _compute_context._shader_bindings->getPipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

private:
    sp<Shader> _shader;
    sp<Buffer> _buffer;

    ComputeContext _compute_context;
};

}

SnippetDrawCompute::SnippetDrawCompute(sp<Shader> shader, sp<Buffer> buffer, sp<Integer> numWorkGroups)
    : _shader(std::move(shader)), _buffer(std::move(buffer)), _num_work_groups(std::move(numWorkGroups)), _shader_bindings(_shader->makeBindings(ModelLoader::RENDER_MODE_NONE, PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS))
{
}

sp<Snippet::DrawEvents> SnippetDrawCompute::makeDrawEvents(const RenderRequest& renderRequest)
{
    return sp<DrawEventsCompute>::make(_shader, _buffer, ComputeContext(_shader_bindings, _shader->snapshot(renderRequest), _buffer->snapshot(), _num_work_groups->val()));
}

SnippetDrawCompute::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)), _buffer(factory.ensureBuilder<Buffer>(manifest, "buffer")),
      _num_work_groups(factory.ensureBuilder<Integer>(manifest, "num-work-groups"))
{
}

sp<Snippet> SnippetDrawCompute::BUILDER::build(const Scope& args)
{
    return sp<SnippetDrawCompute>::make(_shader->build(args), _buffer->build(args), _num_work_groups->build(args));
}

}
