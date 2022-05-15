#include "renderer/impl/render_command_composer/rcc_draw_quads.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/vertices.h"

namespace ark {

RCCDrawQuads::RCCDrawQuads(Model model)
    : _model(std::move(model))
{
}

sp<ShaderBindings> RCCDrawQuads::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _vertices = renderController.makeVertexBuffer();
    _shared_buffer = renderController.getSharedBuffer(renderMode, _model);
    return shader.makeBindings(renderMode, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS);
}

void RCCDrawQuads::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _shared_buffer->snapshot(renderController, snapshot._index_count / 6, snapshot._items.size());
}

sp<RenderCommand> RCCDrawQuads::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    const std::vector<Renderable::Snapshot>& items = snapshot._items;

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    buf.setIndices(snapshot._index_buffer);

    size_t offset = 0;
    bool reload = snapshot._flag == RenderLayer::SNAPSHOT_FLAG_RELOAD || _vertices.size() == 0;
    for(const Renderable::Snapshot& i : items)
    {
        size_t vertexCount = i._model->vertexCount();
        if(reload || i._dirty)
        {
            VertexStream writer = buf.makeVertexStream(renderRequest, vertexCount, offset);
            i._model->writeRenderable(writer, i);
        }
        offset += vertexCount;
    }

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), std::move(snapshot._ssbos),
                                  buf.vertices().toSnapshot(_vertices), buf.indices(), DrawingContext::ParamDrawElements(0, buf.indices().length<element_index_t>()));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

}
