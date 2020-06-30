#include "renderer/impl/renderer/render_pass.h"

#include "core/base/api.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/uploader.h"

namespace ark {

RenderPass::RenderPass(sp<Shader> shader)
    : _shader(shader), _shader_bindings(shader->makeBindings(ModelLoader::RENDER_MODE_TRIANGLES, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS))
{
}

void RenderPass::render(RenderRequest& renderRequest, const V3& position)
{
    DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->snapshot(renderRequest), _vertex_buffer.snapshot(), _index_buffer, DrawingContext::ParamDrawElements(0, _index_buffer.length<element_index_t>()));
    renderRequest.addRequest(drawingContext.toRenderCommand());
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext))
{
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    return sp<RenderPass>::make(_shader->build(args));
}

}
