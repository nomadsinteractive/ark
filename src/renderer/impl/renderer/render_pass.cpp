#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/vertices.h"

namespace ark {

namespace {

Enum::DrawProcedure toDrawProcedure(const Buffer& indexBuffer, const std::map<uint32_t, sp<Uploader>>& dividedUploaders) {
    return indexBuffer ? (dividedUploaders.size() > 0 ? Enum::DRAW_PROCEDURE_DRAW_INSTANCED : Enum::DRAW_PROCEDURE_DRAW_ELEMENTS) : Enum::DRAW_PROCEDURE_DRAW_ARRAYS;
}

}

RenderPass::RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, sp<Integer> drawCount, Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, const std::map<uint32_t, sp<Uploader>>& dividedUploaders)
    : _shader(std::move(shader)), _index_buffer(std::move(indexBuffer)), _draw_count(std::move(drawCount)), _draw_procedure(drawProcedure),
      _pipeline_bindings(_shader->makeBindings(std::move(vertexBuffer), mode, drawProcedure, dividedUploaders))
{
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    uint32_t drawCount = static_cast<uint32_t>(_draw_count->val());
    if(drawCount > 0)
    {
        DrawingParams drawParam;
        const Buffer& vertices = _pipeline_bindings->vertices();
        if(_draw_procedure == Enum::DRAW_PROCEDURE_DRAW_INSTANCED)
        {
            std::vector<std::pair<uint32_t, Buffer::Snapshot>> dividedBufferSnapshots;
            for(const auto& [i, j] : *_pipeline_bindings->streams())
                dividedBufferSnapshots.emplace_back(i, j.snapshot(j.size()));
            drawParam = DrawingParams::DrawElementsInstanced{0, static_cast<uint32_t>(_index_buffer.size() / sizeof(element_index_t)), std::move(dividedBufferSnapshots)};
        }
        else
            drawParam = DrawingParams::DrawElements{0};
        DrawingContext drawingContext({_pipeline_bindings, _shader->takeUBOSnapshot(renderRequest), _shader->takeSSBOSnapshot(renderRequest)}, _pipeline_bindings->attachments(), vertices.snapshot(),
                                      _index_buffer.snapshot(), drawCount, std::move(drawParam));
        renderRequest.addRenderCommand(drawingContext.toRenderCommand(renderRequest));
    }
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER)), _vertex_buffer(factory.ensureBuilder<Buffer>(manifest, "vertex-buffer")),
      _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")), _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count")),
      _mode(Documents::getAttribute<Enum::RenderMode>(manifest, "mode", Enum::RENDER_MODE_TRIANGLES)), _draw_precedure(EnumMap<Enum::DrawProcedure>::instance().toEnumOrDefault(Documents::getAttribute(manifest, "draw-precedure"), Enum::DRAW_PROCEDURE_AUTO))
{
    for(const document& i : manifest->children("buffer"))
    {
        uint32_t divisor = Documents::ensureAttribute<uint32_t>(i, "divisor");
        sp<Builder<Uploader>> uploader = factory.ensureBuilder<Uploader>(i, "uploader");
        _divided_uploaders.insert({divisor, std::move(uploader)});
    }
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    std::map<uint32_t, sp<Uploader>> dividedUploaders;
    for(const auto& [i, j] : _divided_uploaders)
        dividedUploaders.insert({i, j->build(args)});
    Buffer indexBuffer = _index_buffer ? _index_buffer->build(args) : Buffer();
    Enum::DrawProcedure renderPrecedure = _draw_precedure == Enum::DRAW_PROCEDURE_AUTO ? toDrawProcedure(indexBuffer, dividedUploaders) : _draw_precedure;
    return sp<RenderPass>::make(_shader->build(args), _vertex_buffer->build(args), std::move(indexBuffer), _draw_count->build(args), _mode, renderPrecedure, dividedUploaders);
}

}
