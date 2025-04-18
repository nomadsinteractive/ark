#include "renderer/impl/render_command_composer/rcc_draw_elements.h"

#include "graphics/base/render_layer.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

RCCDrawElements::RCCDrawElements(sp<Model> model)
    : _model(std::move(model))
{
}

sp<PipelineBindings> RCCDrawElements::makePipelineBindings(const Shader& shader, RenderController& renderController, enums::DrawMode renderMode)
{
    _primitive_index_buffer = renderController.getSharedPrimitiveIndexBuffer(_model, renderMode == enums::DRAW_MODE_TRIANGLE_STRIP);
    return shader.makeBindings(renderController.makeVertexBuffer(), renderMode, enums::DRAW_PROCEDURE_DRAW_ELEMENTS);
}

DrawingContext RCCDrawElements::compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot)
{
    const size_t verticesCount = _model->vertexCount();
    const Buffer& vertexBuf = snapshot._stub->_pipeline_bindings->vertices();

    DrawingBuffer buf(snapshot._stub->_pipeline_bindings, snapshot._stub->_stride);
    if(snapshot.needsReload())
    {
        VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesCount * snapshot._elements.size(), 0);
        for(const RenderLayerSnapshot::Element& i : snapshot._elements)
            i._snapshot._model->writeRenderable(writer, i._snapshot);
    }
    else
    {
        size_t offset = 0;
        for(const RenderLayerSnapshot::Element& i : snapshot._elements)
        {
            if(i._snapshot._state.has(Renderable::RENDERABLE_STATE_DIRTY))
            {
                VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesCount, offset);
                i._snapshot._model->writeRenderable(writer, i._snapshot);
            }
            offset += verticesCount;
        }
    }

    Buffer::Snapshot vertices = buf.vertices().toSnapshot(vertexBuf);
    Buffer::Snapshot indices = _primitive_index_buffer->snapshot(snapshot._stub->_render_controller, snapshot._elements.size());
    const uint32_t drawCount = static_cast<uint32_t>(indices.length<element_index_t>());
    return snapshot.toDrawingContext(renderRequest, std::move(vertices), std::move(indices), drawCount, DrawingParams::DrawElements{0});
}

}
