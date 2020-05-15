#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_buffer.h"
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

void RCCMultiDrawElementsIndirect::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

sp<RenderCommand> RCCMultiDrawElementsIndirect::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    const std::vector<Renderable::Snapshot>& items = snapshot._items;

    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    sp<Uploader> indirectUploader = nullptr;

    if(snapshot._flag == RenderLayer::SNAPSHOT_FLAG_RELOAD || _vertices.size() == 0)
    {
        _indirect_cmds.clear();
        size_t offset = 0;
        for(const Renderable::Snapshot& i : items)
        {
            IndirectCmd& modelIndirect = _indirect_cmds[i._type];
            if(modelIndirect._snapshot_offsets.empty())
            {
                const MultiModels::ModelInfo& modelInfo = _multi_models->ensure(i._type);
                modelIndirect._command = {static_cast<uint32_t>(modelInfo._model.indices()->length()), 0, static_cast<uint32_t>(modelInfo._index_offset), static_cast<uint32_t>(modelInfo._vertex_offset), 0};
            }

            ++ (modelIndirect._command._instance_count);
            modelIndirect._snapshot_offsets.push_back(offset ++);
        }
        indirectUploader = makeIndirectBufferUploader();
    }

    writeModelMatices(renderRequest, buf, snapshot);

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), _vertices.snapshot(), _indices.snapshot(),
                                  DrawingContext::ParamDrawMultiElementsIndirect(buf.makeDividedBufferSnapshots(), _draw_indirect.snapshot(indirectUploader), _indirect_cmds.size()));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand();
}

sp<Uploader> RCCMultiDrawElementsIndirect::makeIndirectBufferUploader()
{
    std::vector<DrawingContext::DrawElementsIndirectCommand> cmds;
    for(const auto& i : _indirect_cmds)
        cmds.push_back(i.second._command);
    return sp<Uploader::Vector<DrawingContext::DrawElementsIndirectCommand>>::make(std::move(cmds));
}

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    size_t offset = 0;
    for(auto& i : _indirect_cmds)
        for(size_t i : i.second._snapshot_offsets)
        {
            const Renderable::Snapshot& s = snapshot._items.at(i);
            if(s._dirty)
            {
                VertexStream writer = buf.makeDividedVertexStream(renderRequest, 1, offset++, 1);
                writer.next();
                writer.write(MatrixUtil::scale(MatrixUtil::translate(s._transform.toMatrix(), s._position), s._size));
            }
        }
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
