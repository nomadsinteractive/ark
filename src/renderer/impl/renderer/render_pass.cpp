#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/vertices.h"

namespace ark {

RenderPass::RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, ModelLoader::RenderMode mode, sp<Integer> drawCount, std::vector<std::pair<uint32_t, Buffer>> dividedBuffers)
    : _shader(std::move(shader)), _index_buffer(std::move(indexBuffer)), _draw_count(std::move(drawCount)),
      _shader_bindings(_shader->makeBindings(std::move(vertexBuffer), mode, _index_buffer ? (dividedBuffers.size() > 0 ? PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS_INSTANCED : PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS) : PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS)),
      _divided_buffers(std::move(dividedBuffers))
{
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    uint32_t drawCount = static_cast<uint32_t>(_draw_count->val());
    if(drawCount > 0)
    {
        DrawingParams drawParam;
        const Buffer& vertices = _shader_bindings->vertices();
        if(_divided_buffers.size() > 0)
        {
            std::vector<std::pair<uint32_t, Buffer::Snapshot>> dividedBufferSnapshots;
            for(const auto& [i, j] : _divided_buffers)
                dividedBufferSnapshots.emplace_back(i, j.snapshot(j.size()));
            drawParam = DrawingParams::DrawElementsInstanced{0, static_cast<uint32_t>(_index_buffer.size() / sizeof(element_index_t)), std::move(dividedBufferSnapshots)};
        }
        else
            drawParam = DrawingParams::DrawElements{0};
        DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->takeUBOSnapshot(renderRequest), _shader->takeSSBOSnapshot(renderRequest), vertices.snapshot(),
                                      _index_buffer.snapshot(), drawCount, std::move(drawParam));
        renderRequest.addRenderCommand(drawingContext.toRenderCommand(renderRequest));
    }
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)), _vertex_buffer(factory.ensureBuilder<Buffer>(manifest, "vertex-buffer")),
      _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")), _mode(Documents::getAttribute<ModelLoader::RenderMode>(manifest, "mode", ModelLoader::RENDER_MODE_TRIANGLES)),
      _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count")), _render_controller(resourceLoaderContext->renderController())
{
    for(const document& i : manifest->children("buffer"))
    {
        uint32_t divisor = Documents::ensureAttribute<uint32_t>(i, "divisor");
        sp<Builder<Uploader>> uploader = factory.ensureBuilder<Uploader>(i, "uploader");
        _divided_buffer_uploaders.emplace_back(divisor, std::move(uploader));
    }
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    std::vector<std::pair<uint32_t, Buffer>> dividedBuffers;
    for(const auto& [i, j] : _divided_buffer_uploaders)
        dividedBuffers.emplace_back(i, _render_controller->makeVertexBuffer(Buffer::USAGE_DYNAMIC, j->build(args)));
    return sp<RenderPass>::make(_shader->build(args), _vertex_buffer->build(args), _index_buffer ? _index_buffer->build(args) : Buffer(), _mode, _draw_count->build(args), std::move(dividedBuffers));
}

}
