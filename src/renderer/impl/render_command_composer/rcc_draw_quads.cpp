#include "renderer/impl/render_command_composer/rcc_draw_quads.h"

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

RCCDrawQuads::RCCDrawQuads(Model model)
    : _model(std::move(model))
{
}

sp<ShaderBindings> RCCDrawQuads::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _shared_buffer = renderController.getSharedIndices(_model, false);
    return shader.makeBindings(renderController.makeVertexBuffer(), renderMode, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS);
}

void RCCDrawQuads::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    size_t primitiveCount = snapshot._index_count / 6;
    snapshot._index_buffer = _shared_buffer->snapshot(renderController, primitiveCount, primitiveCount);
}

sp<RenderCommand> RCCDrawQuads::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    const Buffer& vertices = snapshot._stub->_shader_bindings->vertices();

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
//    size_t primitiveCount = snapshot._index_count / 6;
//    buf.setIndices(_shared_buffer->snapshot(renderRequest, primitiveCount, primitiveCount));
    buf.setIndices(snapshot._index_buffer);

    size_t offset = 0;
    bool reload = snapshot.needsReload();
    for(const Renderable::Snapshot& i : snapshot._items)
    {
        size_t vertexCount = i._model->vertexCount();
        if(reload || i.getState(Renderable::RENDERABLE_STATE_DIRTY))
        {
            VertexWriter writer = buf.makeVertexWriter(renderRequest, vertexCount, offset);
            i._model->writeRenderable(writer, i);
        }
        offset += vertexCount;
    }

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), std::move(snapshot._ssbos),
                                  buf.vertices().toSnapshot(vertices), buf.indices(), DrawingContext::ParamDrawElements(0, buf.indices().length<element_index_t>()));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

}
