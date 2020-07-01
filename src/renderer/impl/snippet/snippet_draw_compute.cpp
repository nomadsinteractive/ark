#include "renderer/impl/snippet/snippet_draw_compute.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"

namespace ark {

SnippetDrawCompute::SnippetDrawCompute(sp<Shader> shader, sp<Buffer> buffer)
    : _shader(std::move(shader)), _buffer(std::move(buffer)), _shader_bindings(_shader->makeBindings(ModelLoader::RENDER_MODE_NONE, PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS)) {
}

void SnippetDrawCompute::postDraw(GraphicsContext& graphicsContext)
{
    DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), {}, _buffer->snapshot(), Buffer::Snapshot(), DrawingContext::ParamDrawElements(0, 0));
    _shader_bindings->getPipeline(graphicsContext)->compute(graphicsContext, drawingContext);
}

SnippetDrawCompute::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)), _buffer(factory.ensureBuilder<Buffer>(manifest, "buffer"))
{
}

sp<SnippetDraw> SnippetDrawCompute::BUILDER::build(const Scope& args)
{
    return sp<SnippetDrawCompute>::make(_shader->build(args), _buffer->build(args));
}

}
