#include "renderer/impl/render_command_composer/rcc_draw_elements_instanced.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/util/matrix_util.h"

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

RCCDrawElementsInstanced::RCCDrawElementsInstanced(Model model)
    : _model(std::move(model))
{
}

sp<ShaderBindings> RCCDrawElementsInstanced::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _vertices = renderController.makeVertexBuffer();
    _indices = renderController.makeIndexBuffer(Buffer::USAGE_STATIC, sp<Uploader::Array<element_index_t>>::make(_model.indices()));
    return shader.makeBindings(renderMode, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS_INSTANCED);
}

void RCCDrawElementsInstanced::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _indices.snapshot();
}

sp<RenderCommand> RCCDrawElementsInstanced::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    size_t verticesLength = _model.vertices()->length();
    const std::vector<Renderable::Snapshot>& items = snapshot._items;
    const sp<ModelLoader>& modelLoader = snapshot._stub->_model_loader;

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    buf.setIndices(snapshot._index_buffer);

    if(snapshot._flag == RenderLayer::SNAPSHOT_FLAG_RELOAD || _vertices.size() == 0)
    {
        VertexStream writer = buf.makeVertexStream(renderRequest, verticesLength, 0);
        const Model model = modelLoader->load(0);
        model.writeToStream(writer, V3(1.0f));
    }

    VertexStream writer = buf.makeDividedVertexStream(renderRequest, items.size(), 0, 1);
    for(const Renderable::Snapshot& i : items)
    {
        writer.next();
        writer.write(MatrixUtil::scale(MatrixUtil::translate(i._transform.toMatrix(), i._position), i._size));
    }

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos),
                                  buf.vertices().toSnapshot(_vertices), buf.indices(), DrawingContext::ParamDrawElementsInstanced(0, _model.indices()->length(), static_cast<int32_t>(items.size()), buf.makeDividedBufferSnapshots()));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand();
}

}
