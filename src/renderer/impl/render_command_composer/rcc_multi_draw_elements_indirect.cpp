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

RCCMultiDrawElementsIndirect::RCCMultiDrawElementsIndirect(const sp<ModelBundle>& multiModels)
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
    bool reload = snapshot._flag == RenderLayer::SNAPSHOT_FLAG_RELOAD || _vertices.size() == 0;

    if(reload)
    {
        _indirect_cmds.clear();
        size_t offset = 0;
        for(const Renderable::Snapshot& i : items)
        {
            IndirectCmd& modelIndirect = _indirect_cmds[i._type];
            if(modelIndirect._snapshot_offsets.empty())
            {
                const ModelBundle::ModelInfo& modelInfo = _multi_models->ensure(i._type);
                modelIndirect._command = {static_cast<uint32_t>(modelInfo._model.indexLength()), 0, static_cast<uint32_t>(modelInfo._index_offset), static_cast<uint32_t>(modelInfo._vertex_offset), 0};
            }

            ++ (modelIndirect._command._instance_count);
            modelIndirect._snapshot_offsets.push_back(offset ++);
        }
        indirectUploader = makeIndirectBufferUploader();
    }

    writeModelMatices(renderRequest, buf, snapshot, reload);

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), _vertices.snapshot(), _indices.snapshot(),
                                  DrawingContext::ParamDrawMultiElementsIndirect(buf.makeDividedBufferSnapshots(), _draw_indirect.snapshot(indirectUploader), static_cast<uint32_t>(_indirect_cmds.size())));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand();
}

sp<Uploader> RCCMultiDrawElementsIndirect::makeIndirectBufferUploader()
{
    std::vector<DrawingContext::DrawElementsIndirectCommand> cmds;
    uint32_t baseInstance = 0;
    for(const auto& i : _indirect_cmds)
    {
        const DrawingContext::DrawElementsIndirectCommand& cmd = i.second._command;
        cmds.push_back(i.second._command);
        cmds.back()._base_instance = baseInstance;
        baseInstance += cmd._instance_count;
    }
    return sp<Uploader::Vector<DrawingContext::DrawElementsIndirectCommand>>::make(std::move(cmds));
}

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot, bool reload)
{
    size_t offset = 0;
    for(const auto& i : _indirect_cmds)
        for(size_t j : i.second._snapshot_offsets)
        {
            const Renderable::Snapshot& s = snapshot._items.at(j);
            if(reload || s._dirty)
            {
                VertexStream writer = buf.makeDividedVertexStream(renderRequest, 1, offset, 1);
                writer.next();
                writer.write(MatrixUtil::translate(M4::identity(), s._position) * MatrixUtil::scale(s._transform.toMatrix(), s._size));
            }
            ++ offset;
        }
}

RCCMultiDrawElementsIndirect::VerticesUploader::VerticesUploader(const sp<ModelBundle>& multiModels, const sp<PipelineInput>& pipelineInput)
    : Uploader(multiModels->vertexLength() * pipelineInput->getStream(0).stride()), _multi_models(multiModels), _pipeline_input(pipelineInput)
{
}

void RCCMultiDrawElementsIndirect::VerticesUploader::upload(Writable& uploader)
{
    uint32_t offset = 0;
    size_t stride = _pipeline_input->getStream(0).stride();
    Buffer::Attributes attributes(_pipeline_input);
    for(const auto& i : _multi_models->models())
    {
        const Model& model = i.second._model;
        uint32_t size = static_cast<uint32_t>(model.vertices()->length() * stride);
        uint8_t* buf = new uint8_t[size];
        VertexStream stream(attributes, false, buf, size, stride);
        i.second._model.writeToStream(stream, V3(1.0f));
        uploader.write(buf, size, offset);
        offset += size;
        delete[] buf;
    }
}

RCCMultiDrawElementsIndirect::IndicesUploader::IndicesUploader(const sp<ModelBundle>& multiModels)
    : Uploader(multiModels->indexLength() * sizeof(element_index_t)), _multi_models(multiModels)
{
}

namespace {

class WritableWithOffset : public Writable {
public:
    WritableWithOffset(Writable& delegate, uint32_t offset)
        : _delegate(delegate), _offset(offset) {
    }

    virtual uint32_t write(void* buffer, uint32_t size, uint32_t offset) override {
        _delegate.write(buffer, size, _offset + offset);
        return size;
    }

private:
    Writable& _delegate;
    uint32_t _offset;
};

}

void RCCMultiDrawElementsIndirect::IndicesUploader::upload(Writable& uploader)
{
    uint32_t offset = 0;
    for(const auto& i: _multi_models->models())
    {
        const sp<Uploader>& indices = i.second._model.indices();
        uint32_t size = static_cast<uint32_t>(indices->size());
        WritableWithOffset writable(uploader, offset);
        indices->upload(writable);
        offset += size;
    }
}

}
