#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

Enum::DrawProcedure toDrawProcedure(const Buffer& indexBuffer, const Buffer& indirectBuffer, const Vector<std::pair<uint32_t, sp<Uploader>>>& dividedUploaders) {
    if(indirectBuffer)
        return Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT;
    return indexBuffer ? (dividedUploaders.size() > 0 ? Enum::DRAW_PROCEDURE_DRAW_INSTANCED : Enum::DRAW_PROCEDURE_DRAW_ELEMENTS) : Enum::DRAW_PROCEDURE_DRAW_ARRAYS;
}

}

RenderPass::RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, sp<Integer> drawCount, const Enum::DrawMode drawMode, const Enum::DrawProcedure drawProcedure, const Vector<std::pair<uint32_t, sp<Uploader>>>& dividedUploaders, Buffer indirectBuffer)
    : _shader(std::move(shader)), _index_buffer(std::move(indexBuffer)), _draw_count(std::move(drawCount)), _draw_procedure(drawProcedure), _indirect_buffer(std::move(indirectBuffer)),
      _pipeline_bindings(_shader->makeBindings(std::move(vertexBuffer), drawMode, drawProcedure, dividedUploaders))
{
    CHECK(drawProcedure != Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT || indirectBuffer, "Must provide an indirectBuffer in DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT rendering mode");
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/, const sp<DrawDecorator>& drawDecorator)
{
    if(const uint32_t drawCount = static_cast<uint32_t>(_draw_count->val()); drawCount > 0)
    {
        DrawingParams drawParam = DrawingParams::DrawElements{0};
        switch(_draw_procedure)
        {
            case Enum::DRAW_PROCEDURE_DRAW_ARRAYS:
                break;
            case Enum::DRAW_PROCEDURE_DRAW_ELEMENTS:
                break;
            case Enum::DRAW_PROCEDURE_DRAW_INSTANCED:
            {
                Vector<std::pair<uint32_t, Buffer::Snapshot>> dividedBufferSnapshots;
                for(const auto& [i, j] : _pipeline_bindings->streams())
                    dividedBufferSnapshots.emplace_back(i, j.snapshot(j.size()));
                drawParam = DrawingParams::DrawElementsInstanced{0, static_cast<uint32_t>(_index_buffer.size() / sizeof(element_index_t)), std::move(dividedBufferSnapshots)};
                break;
            }
            case Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT:
            {
                Vector<std::pair<uint32_t, Buffer::Snapshot>> dividedBufferSnapshots;
                for(const auto& [i, j] : _pipeline_bindings->streams())
                    dividedBufferSnapshots.emplace_back(i, j.snapshot(j.size()));
                drawParam = DrawingParams::DrawMultiElementsIndirect{std::move(dividedBufferSnapshots), _indirect_buffer.snapshot(), drawCount};
                break;
            }
            default:
                FATAL("Unknown draw procedure: %d", _draw_procedure);
                break;
        }
        DrawingContext drawingContext(_pipeline_bindings, _shader->takeBufferSnapshot(renderRequest, false), _pipeline_bindings->vertices().snapshot(),
                                      _index_buffer.snapshot(), drawCount, std::move(drawParam));
        renderRequest.addRenderCommand(drawingContext.toRenderCommand(renderRequest, drawDecorator));
    }
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER)), _vertex_buffer(factory.ensureBuilder<Buffer>(manifest, "vertex-buffer")), _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")),
      _indirect_buffer(factory.getBuilder<Buffer>(manifest, "indirect-buffer")), _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count")),
      _mode(Documents::getAttribute<Enum::DrawMode>(manifest, "mode", Enum::DRAW_MODE_TRIANGLES)), _draw_precedure(Documents::getAttribute<Enum::DrawProcedure>(manifest, "draw-precedure", Enum::DRAW_PROCEDURE_AUTO))
{
    for(const document& i : manifest->children("buffer"))
    {
        const uint32_t divisor = Documents::ensureAttribute<uint32_t>(i, "divisor");
        _divided_uploaders.emplace_back(divisor, factory.ensureBuilder<Uploader>(i, constants::UPLOADER));
    }
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    Vector<std::pair<uint32_t, sp<Uploader>>> dividedUploaders;
    for(const auto& [i, j] : _divided_uploaders)
        dividedUploaders.emplace_back(i, j->build(args));

    Buffer indexBuffer = _index_buffer ? std::move(*_index_buffer->build(args)) : Buffer();
    Buffer indirectBuffer = _indirect_buffer ? std::move(*_indirect_buffer->build(args)) : Buffer();
    Enum::DrawProcedure renderPrecedure = _draw_precedure == Enum::DRAW_PROCEDURE_AUTO ? toDrawProcedure(indexBuffer, indirectBuffer, dividedUploaders) : _draw_precedure;
    return sp<RenderPass>::make(_shader->build(args), _vertex_buffer->build(args), std::move(indexBuffer), _draw_count->build(args), _mode, renderPrecedure, dividedUploaders);
}

}
