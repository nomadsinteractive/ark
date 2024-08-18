#include "renderer/impl/render_command_composer/rcc_draw_elements.h"

#include "graphics/base/render_layer.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

RCCDrawElements::RCCDrawElements(sp<Model> model)
    : _model(std::move(model))
{
}

sp<PipelineBindings> RCCDrawElements::makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode)
{
    _primitive_index_buffer = renderController.getSharedPrimitiveIndexBuffer(_model, renderMode == Enum::RENDER_MODE_TRIANGLE_STRIP);
    return shader.makeBindings(renderController.makeVertexBuffer(), renderMode, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS);
}

sp<RenderCommand> RCCDrawElements::compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot)
{
    const size_t verticesCount = _model->vertexCount();
    const Buffer& vertices = snapshot._stub->_pipeline_bindings->vertices();

    DrawingBuffer buf(snapshot._stub->_pipeline_bindings, snapshot._stub->_stride);
    buf.setIndices(_primitive_index_buffer->snapshot(snapshot._stub->_render_controller, snapshot._droplets.size()));

    if(snapshot.needsReload())
    {
        VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesCount * snapshot._droplets.size(), 0);
        for(const RenderLayerSnapshot::Droplet& i : snapshot._droplets)
            i._snapshot._model->writeRenderable(writer, i._snapshot);
    }
    else
    {
        size_t offset = 0;
        for(const RenderLayerSnapshot::Droplet& i : snapshot._droplets)
        {
            if(i._snapshot._state.has(Renderable::RENDERABLE_STATE_DIRTY))
            {
                VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesCount, offset);
                i._snapshot._model->writeRenderable(writer, i._snapshot);
            }
            offset += verticesCount;
        }
    }

    DrawingContext drawingContext(snapshot._stub->_pipeline_bindings, snapshot._buffer_object, snapshot._stub->_pipeline_bindings->attachments(),
                                  buf.vertices().toSnapshot(vertices), buf.indices(), static_cast<uint32_t>(buf.indices().length<element_index_t>()), DrawingParams::DrawElements{0});

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererRect(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

}
