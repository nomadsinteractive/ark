#include "renderer/impl/render_command_composer/rcc_draw_elements_instanced.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/render_controller.h"
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

sp<PipelineBindings> RCCDrawElementsInstanced::makePipelineBindings(const Shader& shader, RenderController& renderController, enums::DrawMode renderMode)
{
    _indices = renderController.makeIndexBuffer({}, _model.indices());
    return shader.makeBindings(renderController.makeVertexBuffer(), renderMode, enums::DRAW_PROCEDURE_DRAW_INSTANCED);
}

DrawingContext RCCDrawElementsInstanced::compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot)
{
    const size_t verticesLength = _model.vertices()->length();
    const sp<ModelLoader>& modelLoader = snapshot._stub->_model_loader;
    const Buffer& vertices = snapshot._stub->_pipeline_bindings->vertices();

    DrawingBuffer buf(snapshot._stub->_pipeline_bindings, snapshot._stub->_stride);
    if(snapshot.needsReload())
    {
        VertexWriter writer = buf.makeVertexWriter(renderRequest, verticesLength, 0);
        const Model model = modelLoader->loadModel(0);
        model.writeToStream(writer, V3(1.0f));
    }

    const PipelineLayout::VertexDescriptor& attributeOffsets = buf.pipelineBindings()->pipelineDescriptor()->vertexDescriptor();
    const size_t attributeStride = attributeOffsets._stride;
    const bool hasModelMatrix = attributeOffsets._offsets[Attribute::USAGE_MODEL_MATRIX] != -1;

    VertexWriter writer = buf.makeDividedVertexWriter(renderRequest, snapshot._elements.size(), 0, 1);
    for(const RenderLayerSnapshot::Element& i : snapshot._elements)
    {
        const Renderable::Snapshot& snapshot = i._snapshot;
        writer.next();
        if(hasModelMatrix)
            writer.writeAttribute(MatrixUtil::translate({}, snapshot._position) * MatrixUtil::scale(snapshot._transform->toMatrix(snapshot._transform_snapshot), snapshot._size), Attribute::USAGE_MODEL_MATRIX);
        ByteArray::Borrowed divided = snapshot._varyings_snapshot.getDivided(1)._content;
        if(divided.length() > attributeStride)
            writer.write(divided.buf() + attributeStride, divided.length() - attributeStride, attributeStride);
    }

    return snapshot.toDrawingContext(renderRequest, buf.vertices().toSnapshot(vertices), _indices.snapshot(), static_cast<uint32_t>(snapshot._elements.size()), DrawingParams::DrawElementsInstanced{0, static_cast<uint32_t>(_model.indexCount()), buf.toDividedBufferSnapshots()});
}

}
