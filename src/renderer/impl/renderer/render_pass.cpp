#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

RenderPass::RenderPass(sp<Shader> shader, sp<Buffer> vertexBuffer, sp<Buffer> indexBuffer, sp<Integer> drawCount)
    : _shader(std::move(shader)), _vertex_buffer(std::move(vertexBuffer)), _index_buffer(std::move(indexBuffer)), _draw_count(std::move(drawCount)),
      _shader_bindings(_shader->makeBindings(ModelLoader::RENDER_MODE_POINTS, PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS))
{
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->snapshot(renderRequest), _vertex_buffer->snapshot(),
                                  _index_buffer ? _index_buffer->snapshot() : Buffer::Snapshot(), DrawingContext::ParamDrawElements(0, static_cast<uint32_t>(_draw_count->val())));
    renderRequest.addRequest(drawingContext.toRenderCommand(renderRequest));
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)),
      _vertex_buffer(factory.ensureBuilder<Buffer>(manifest, "vertex-buffer")), _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")),
      _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count"))
{
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    return sp<RenderPass>::make(_shader->build(args), _vertex_buffer->build(args), _index_buffer->build(args), _draw_count->build(args));
}

}
