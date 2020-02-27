#include "renderer/impl/render_command_composer/rcc_instanced_models.h"

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

RCCInstancedModels::RCCInstancedModels(Model model)
    : _model(std::move(model)), _attachments(sp<ByType>::make())
{
}

sp<ShaderBindings> RCCInstancedModels::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _indices = renderController.makeIndexBuffer(Buffer::USAGE_STATIC, sp<Uploader::Array<element_index_t>>::make(_model.indices()));
    return shader.makeBindings(renderMode);
}

void RCCInstancedModels::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _indices.snapshot();
}

sp<RenderCommand> RCCInstancedModels::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    size_t verticesLength = _model.vertices()->length();
    const std::vector<Renderable::Snapshot>& items = snapshot._items;
    const sp<ModelLoader>& modelLoader = snapshot._stub->_model_loader;

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    buf.setIndices(snapshot._index_buffer);

    if(snapshot._flag == RenderLayer::SNAPSHOT_FLAG_RELOAD)
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

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, _attachments, std::move(snapshot._ubos),
                                  buf.vertices().toSnapshot(snapshot._stub->_vertices), buf.indices(), static_cast<int32_t>(items.size()));

    if(snapshot._stub->_scissor)
        drawingContext._parameters._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    drawingContext._instanced_array_snapshots = buf.makeDividedBufferSnapshots();

    return drawingContext.toRenderCommand();
}

}
