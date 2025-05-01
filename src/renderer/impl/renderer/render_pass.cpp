#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

enums::DrawProcedure toDrawProcedure(const Buffer& indexBuffer, const Buffer& indirectBuffer, const Vector<std::pair<uint32_t, Buffer>>& instanceBuffers) {
    if(indirectBuffer)
        return enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT;
    return indexBuffer ? (instanceBuffers.size() > 0 ? enums::DRAW_PROCEDURE_DRAW_INSTANCED : enums::DRAW_PROCEDURE_DRAW_ELEMENTS) : enums::DRAW_PROCEDURE_DRAW_ARRAYS;
}

}

RenderPass::RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, sp<Integer> offset, sp<Integer> drawCount, const enums::DrawMode drawMode, const enums::DrawProcedure drawProcedure, Vector<std::pair<uint32_t, Buffer>> instanceBuffers, Buffer indirectBuffer)
    : _shader(std::move(shader)), _index_buffer(std::move(indexBuffer)), _offset(std::move(offset)), _draw_count(std::move(drawCount)), _draw_procedure(drawProcedure), _indirect_buffer(std::move(indirectBuffer)),
      _pipeline_bindings(_shader->makeBindings(std::move(vertexBuffer), drawMode, drawProcedure, std::move(instanceBuffers)))
{
    CHECK(drawProcedure != enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT || indirectBuffer, "Must provide an indirectBuffer in DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT rendering mode");
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/, const sp<DrawDecorator>& drawDecorator)
{
    const uint32_t drawCount = static_cast<uint32_t>(_draw_count->val());
    DrawingParams drawParam = DrawingParams::DrawElements{0};
    switch(_draw_procedure)
    {
        case enums::DRAW_PROCEDURE_DRAW_ARRAYS:
            break;
        case enums::DRAW_PROCEDURE_DRAW_ELEMENTS:
            break;
        case enums::DRAW_PROCEDURE_DRAW_INSTANCED:
        {
            drawParam = DrawingParams::DrawElementsInstanced{static_cast<uint32_t>(_offset.val()), static_cast<uint32_t>(_index_buffer.size() / sizeof(element_index_t)), _pipeline_bindings->makeInstanceBufferSnapshots()};
            break;
        }
        case enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT:
        {
            drawParam = DrawingParams::DrawMultiElementsIndirect{_pipeline_bindings->makeInstanceBufferSnapshots(), _indirect_buffer.snapshot(), drawCount};
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

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, constants::SHADER)), _vertex_buffer(factory.ensureBuilder<Buffer>(manifest, "vertex-buffer")), _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")),
      _indirect_buffer(factory.getBuilder<Buffer>(manifest, "indirect-buffer")), _offset(factory.getBuilder<Integer>(manifest, "offset")), _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count")),
      _mode(Documents::getAttribute<enums::DrawMode>(manifest, "mode", enums::DRAW_MODE_TRIANGLES)), _draw_precedure(Documents::getAttribute<enums::DrawProcedure>(manifest, "draw-precedure", enums::DRAW_PROCEDURE_AUTO))
{
    for(const document& i : manifest->children("buffer"))
    {
        const uint32_t divisor = Documents::ensureAttribute<uint32_t>(i, "divisor");
        _instance_buffers.emplace_back(divisor, factory.ensureBuilder<Buffer>(i));
    }
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    Vector<std::pair<uint32_t, Buffer>> instanceBuffers;
    for(const auto& [i, j] : _instance_buffers)
        instanceBuffers.emplace_back(i, std::move(*j->build(args)));

    Buffer indexBuffer = _index_buffer ? std::move(*_index_buffer->build(args)) : Buffer();
    Buffer indirectBuffer = _indirect_buffer ? std::move(*_indirect_buffer->build(args)) : Buffer();
    enums::DrawProcedure renderPrecedure = _draw_precedure == enums::DRAW_PROCEDURE_AUTO ? toDrawProcedure(indexBuffer, indirectBuffer, instanceBuffers) : _draw_precedure;
    return sp<Renderer>::make<RenderPass>(_shader->build(args), _vertex_buffer->build(args), std::move(indexBuffer), _offset.build(args), _draw_count->build(args), _mode, renderPrecedure, instanceBuffers, std::move(indirectBuffer));
}

}
