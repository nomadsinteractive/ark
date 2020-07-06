#include "renderer/impl/snippet/snippet_draw_compute.h"

#include "renderer/base/compute_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class DrawEventsCompute : public Snippet::DrawEvents {
public:
    DrawEventsCompute(const sp<Shader>& shader, const sp<Buffer>& buffer, const sp<ShaderBindings>& shaderBindings, const RenderRequest& renderRequest)
        : _shader(shader), _buffer(buffer), _shader_bindings(shaderBindings), _compute_context(_shader_bindings, _shader->snapshot(renderRequest), _buffer->snapshot()) {
    }

    virtual void preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
    }

    virtual void postDraw(GraphicsContext& graphicsContext) override {
        _shader_bindings->getPipeline(graphicsContext)->compute(graphicsContext, _compute_context);
    }

private:
    sp<Shader> _shader;
    sp<Buffer> _buffer;
    sp<ShaderBindings> _shader_bindings;

    ComputeContext _compute_context;
};

}

SnippetDrawCompute::SnippetDrawCompute(sp<Shader> shader, sp<Buffer> buffer)
    : _shader(std::move(shader)), _buffer(std::move(buffer)), _shader_bindings(_shader->makeBindings(ModelLoader::RENDER_MODE_NONE, PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS))
{
}

sp<Snippet::DrawEvents> SnippetDrawCompute::makeDrawEvents(const RenderRequest& renderRequest)
{
    return sp<DrawEventsCompute>::make(_shader, _buffer, _shader_bindings, renderRequest);
}

SnippetDrawCompute::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)), _buffer(factory.ensureBuilder<Buffer>(manifest, "buffer"))
{
}

sp<Snippet> SnippetDrawCompute::BUILDER::build(const Scope& args)
{
    return sp<SnippetDrawCompute>::make(_shader->build(args), _buffer->build(args));
}

}
