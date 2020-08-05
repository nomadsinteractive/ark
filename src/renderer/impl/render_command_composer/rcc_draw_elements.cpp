#include "renderer/impl/render_command_composer/rcc_draw_elements.h"

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

RCCDrawElements::RCCDrawElements(Model model)
    : _model(std::move(model))
{
}

sp<ShaderBindings> RCCDrawElements::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _vertices = renderController.makeVertexBuffer();
    _shared_buffer = renderController.getSharedBuffer(renderMode, _model);
    return shader.makeBindings(renderMode, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS);
}

void RCCDrawElements::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _shared_buffer->snapshot(renderController, snapshot._items.size(), snapshot._items.size());
}

sp<RenderCommand> RCCDrawElements::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    size_t verticesLength = _model.vertices()->length();
    const std::vector<Renderable::Snapshot>& items = snapshot._items;
    const sp<ModelLoader>& modelLoader = snapshot._stub->_model_loader;

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    buf.setIndices(snapshot._index_buffer);

    if(snapshot._flag == RenderLayer::SNAPSHOT_FLAG_RELOAD || _vertices.size() == 0)
    {
        VertexStream writer = buf.makeVertexStream(renderRequest, verticesLength * items.size(), 0);
        for(const Renderable::Snapshot& i : items)
        {
            const Model model = modelLoader->loadModel(i._type);
            writer.setRenderObject(i);
            model.writeToStream(writer, i._size);
        }
    }
    else
    {
        size_t offset = 0;
        for(const Renderable::Snapshot& i : items)
        {
            if(i._dirty)
            {
                VertexStream writer = buf.makeVertexStream(renderRequest, verticesLength, offset);
                const Model model = modelLoader->loadModel(i._type);
                writer.setRenderObject(i);
                model.writeToStream(writer, i._size);
            }
            offset += verticesLength;
        }
    }

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), std::move(snapshot._ssbos),
                                  buf.vertices().toSnapshot(_vertices), buf.indices(), DrawingContext::ParamDrawElements(0, buf.indices().length<element_index_t>()));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

}
