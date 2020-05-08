#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/multi_models.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/vertices.h"

namespace ark {

RCCMultiDrawElementsIndirect::RCCMultiDrawElementsIndirect(const sp<MultiModels>& multiModels)
    : _multi_models(multiModels)
{
}

sp<ShaderBindings> RCCMultiDrawElementsIndirect::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _vertices = renderController.makeVertexBuffer(Buffer::USAGE_STATIC, sp<VerticesUploader>::make(_multi_models, shader.input()));
    _indices = renderController.makeIndexBuffer(Buffer::USAGE_STATIC, sp<IndicesUploader>::make(_multi_models));
    _draw_indirect = renderController.makeBuffer(Buffer::TYPE_DRAW_INDIRECT, Buffer::USAGE_DYNAMIC, nullptr);
    return shader.makeBindings(renderMode, PipelineBindings::RENDER_PROCEDURE_DRAW_MULTI_ELEMENTS_INDIRECT);
}

void RCCMultiDrawElementsIndirect::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _indices.snapshot();
}

sp<RenderCommand> RCCMultiDrawElementsIndirect::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    const std::vector<Renderable::Snapshot>& items = snapshot._items;

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    VertexStream writer = buf.makeDividedVertexStream(renderRequest, items.size(), 0, 1);

    std::vector<DrawingContext::DrawElementsIndirectCommand> indirectCmds;
    for(const Renderable::Snapshot& i : items)
    {
        writer.next();
        writer.write(MatrixUtil::scale(MatrixUtil::translate(i._transform.toMatrix(), i._position), i._size));
    }

    const Model model = _multi_models->load(1);
    indirectCmds.push_back({static_cast<uint32_t>(model.indices()->length()), static_cast<uint32_t>(items.size()), 0, 0, 0});

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), _vertices.snapshot(), _indices.snapshot(),
                                  DrawingContext::ParamDrawMultiElementsIndirect(buf.makeDividedBufferSnapshots(), _draw_indirect.snapshot(sp<Uploader::Vector<DrawingContext::DrawElementsIndirectCommand>>::make(std::move(indirectCmds)))));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand();
}

RCCMultiDrawElementsIndirect::VerticesUploader::VerticesUploader(const sp<MultiModels>& multiModels, const sp<PipelineInput>& pipelineInput)
    : Uploader(multiModels->vertexLength() * pipelineInput->getStream(0).stride()), _multi_models(multiModels), _pipeline_input(pipelineInput)
{
}

void RCCMultiDrawElementsIndirect::VerticesUploader::upload(const Uploader::UploadFunc& uploader)
{
    size_t offset = 0;
    size_t stride = _pipeline_input->getStream(0).stride();
    Buffer::Attributes attributes(_pipeline_input);
    for(const auto& i : _multi_models->models())
    {
        const Model& model = i.second._model;
        size_t size = model.vertices()->length() * stride;
        uint8_t* buf = new uint8_t[size];
        VertexStream stream(attributes, buf, size, stride, false);
        i.second._model.writeToStream(stream, V3(1.0f));
        uploader(buf, size, offset);
        offset += size;
        delete[] buf;
    }
}

RCCMultiDrawElementsIndirect::IndicesUploader::IndicesUploader(const sp<MultiModels>& multiModels)
    : Uploader(multiModels->indexLength() * sizeof(element_index_t)), _multi_models(multiModels)
{
}

void RCCMultiDrawElementsIndirect::IndicesUploader::upload(const Uploader::UploadFunc& uploader)
{
    size_t offset = 0;
    for(const auto& i: _multi_models->models())
    {
        const array<element_index_t>& indices = i.second._model.indices();
        size_t size = indices->size();
        uploader(indices->buf(), size, offset);
        offset += size;
    }
}

}
