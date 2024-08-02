#include "renderer/impl/render_command_composer/rcc_draw_elements_instanced.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/vertices.h"

namespace ark {

RCCDrawElementsInstanced::RCCDrawElementsInstanced(Model model)
    : _model(std::move(model))
{
}

sp<PipelineBindings> RCCDrawElementsInstanced::makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode)
{
    _indices = renderController.makeIndexBuffer(Buffer::USAGE_STATIC, _model.indices());
    return shader.makeBindings(renderController.makeVertexBuffer(), renderMode, Enum::DRAW_PROCEDURE_DRAW_INSTANCED);
}

void RCCDrawElementsInstanced::postSnapshot(RenderController& /*renderController*/, RenderLayerSnapshot& snapshot)
{
    snapshot._index_buffer = _indices.snapshot();
}

sp<RenderCommand> RCCDrawElementsInstanced::compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot)
{
    const size_t verticesLength = _model.vertices()->length();
    const sp<ModelLoader>& modelLoader = snapshot._stub->_model_loader;
    const Buffer& vertices = snapshot._stub->_pipeline_bindings->vertices();

    DrawingBuffer buf(snapshot._stub->_pipeline_bindings, snapshot._stub->_stride);
    buf.setIndices(snapshot._index_buffer);

    if(snapshot.needsReload())
    {
        VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesLength, 0);
        const Model model = modelLoader->loadModel(0);
        model.writeToStream(writer, V3(1.0f));
    }

    VertexWriter writer = buf.makeDividedVertexWriter(renderRequest, snapshot._droplets.size(), 0, 1);
    for(const RenderLayerSnapshot::Droplet& i : snapshot._droplets)
    {
        const Renderable::Snapshot& snapshot = i._snapshot;
        writer.next();
        writer.write(MatrixUtil::translate(M4::identity(), snapshot._position) * MatrixUtil::scale(snapshot._transform.toMatrix(), snapshot._size));
        ByteArray::Borrowed vm = snapshot._varyings.getDivided(0)._content;
        if(vm.length() > 0)
            writer.write(vm.buf() + sizeof(M4), vm.length() - sizeof(M4), sizeof(M4));
    }

    DrawingContext drawingContext({snapshot._stub->_pipeline_bindings, std::move(snapshot._ubos), std::move(snapshot._ssbos)}, snapshot._stub->_pipeline_bindings->attachments(),
                                  buf.vertices().toSnapshot(vertices), buf.indices(), static_cast<uint32_t>(snapshot._droplets.size()), DrawingParams::DrawElementsInstanced{0, static_cast<uint32_t>(_model.indexCount()), buf.toDividedBufferSnapshots()});

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererRect(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

}
