#include "renderer/impl/render_command_composer/rcc_draw_elements.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/shared_indices.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/vertices.h"

namespace ark {

RCCDrawElements::RCCDrawElements(Model model)
    : _model(std::move(model))
{
}

sp<ShaderBindings> RCCDrawElements::makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode)
{
    _shared_buffer = renderController.getSharedIndices(_model, renderMode == Enum::RENDER_MODE_TRIANGLE_STRIP);
    return shader.makeBindings(renderController.makeVertexBuffer(), renderMode, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS);
}

void RCCDrawElements::postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot)
{
    snapshot._index_buffer = _shared_buffer->snapshot(renderController, snapshot._droplets.size(), snapshot._droplets.size());
}

sp<RenderCommand> RCCDrawElements::compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot)
{
    size_t verticesLength = _model.vertices()->length();
    const Buffer& vertices = snapshot._stub->_shader_bindings->vertices();

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    buf.setIndices(snapshot._index_buffer);

    if(snapshot.needsReload())
    {
        VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesLength * snapshot._droplets.size(), 0);
        for(const RenderLayerSnapshot::Droplet& i : snapshot._droplets)
            i._snapshot._model->writeRenderable(writer, i._snapshot);
    }
    else
    {
        size_t offset = 0;
        for(const RenderLayerSnapshot::Droplet& i : snapshot._droplets)
        {
            if(i._snapshot._state.hasState(Renderable::RENDERABLE_STATE_DIRTY))
            {
                VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesLength, offset);
                i._snapshot._model->writeRenderable(writer, i._snapshot);
            }
            offset += verticesLength;
        }
    }

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), std::move(snapshot._ssbos),
                                  buf.vertices().toSnapshot(vertices), buf.indices(), static_cast<uint32_t>(buf.indices().length<element_index_t>()), DrawingParams::DrawElements{0});

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererRect(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

}
