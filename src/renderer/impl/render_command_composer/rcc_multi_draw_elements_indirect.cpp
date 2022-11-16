#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/model_bundle.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/vertices.h"

namespace ark {

RCCMultiDrawElementsIndirect::RCCMultiDrawElementsIndirect(sp<ModelBundle> multiModels)
    : _model_bundle(std::move(multiModels))
{
}

sp<ShaderBindings> RCCMultiDrawElementsIndirect::makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode)
{
    _indices = renderController.makeIndexBuffer(Buffer::USAGE_STATIC, sp<IndicesUploader>::make(_model_bundle));
    _draw_indirect = renderController.makeBuffer(Buffer::TYPE_DRAW_INDIRECT, Buffer::USAGE_DYNAMIC, nullptr);
    return shader.makeBindings(renderController.makeVertexBuffer(Buffer::USAGE_STATIC, sp<VerticesUploader>::make(_model_bundle, shader.input())), renderMode, PipelineBindings::RENDER_PROCEDURE_DRAW_MULTI_ELEMENTS_INDIRECT);
}

void RCCMultiDrawElementsIndirect::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

sp<RenderCommand> RCCMultiDrawElementsIndirect::compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot)
{
    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    const Buffer& vertices = snapshot._stub->_shader_bindings->vertices();
    bool reload = snapshot.needsReload();

    if(reload)
    {
        _indirect_cmds.clear();
        size_t offset = 0;
        for(const Renderable::Snapshot& i : snapshot._items)
        {
            _indirect_cmds.has(i._type);
            IndirectCmd& modelIndirect = _indirect_cmds[i._type];
            if(modelIndirect._snapshot_offsets.empty())
            {
                const ModelBundle::ModelInfo& modelInfo = _model_bundle->ensureModelInfo(i._type);
                modelIndirect._command = {static_cast<uint32_t>(modelInfo._model->indexCount()), 0, static_cast<uint32_t>(modelInfo._index_offset), static_cast<uint32_t>(modelInfo._vertex_offset), 0};
            }

            ++ (modelIndirect._command._instance_count);
            modelIndirect._snapshot_offsets.push_back(offset ++);
        }
    }

    writeModelMatices(renderRequest, buf, snapshot, reload);

    DrawingContext drawingContext(snapshot._stub->_shader_bindings, snapshot._stub->_shader_bindings->attachments(), std::move(snapshot._ubos), std::move(snapshot._ssbos),
                                  vertices.snapshot(), _indices.snapshot(),
                                  DrawingContext::ParamDrawMultiElementsIndirect(buf.toDividedBufferSnapshots(), reload ? _draw_indirect.snapshot(makeIndirectBuffer(renderRequest)) : _draw_indirect.snapshot(), static_cast<uint32_t>(_indirect_cmds.size())));

    if(snapshot._stub->_scissor)
        drawingContext._scissor = snapshot._stub->_render_controller->renderEngine()->toRendererScissor(snapshot._scissor);

    return drawingContext.toRenderCommand(renderRequest);
}

ByteArray::Borrowed RCCMultiDrawElementsIndirect::makeIndirectBuffer(const RenderRequest& renderRequest) const
{
    ByteArray::Borrowed cmds = renderRequest.allocator().sbrk(_indirect_cmds.size() * sizeof(DrawingContext::DrawElementsIndirectCommand));
    DrawingContext::DrawElementsIndirectCommand* pcmds = reinterpret_cast<DrawingContext::DrawElementsIndirectCommand*>(cmds.buf());
    uint32_t baseInstance = 0;
    for(const auto& i : _indirect_cmds)
    {
        *pcmds = i.second._command;
        pcmds->_base_instance = baseInstance;
        baseInstance += pcmds->_instance_count;
         ++ pcmds;
    }
    return cmds;
}

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot, bool reload)
{
    size_t offset = 0;
    for(const auto& i : _indirect_cmds)
        for(size_t j : i.second._snapshot_offsets)
        {
            const Renderable::Snapshot& s = snapshot._items.at(j);
            if(reload || s.getState(Renderable::RENDERABLE_STATE_DIRTY))
            {
                const ModelBundle::ModelInfo& modelInfo = _model_bundle->ensureModelInfo(s._type);
                const Metrics& metrics = modelInfo._model->metrics();
                VertexWriter writer = buf.makeDividedVertexWriter(renderRequest, 1, offset, 1);
                writer.next();
                writer.write(MatrixUtil::translate(M4::identity(), s._position) * MatrixUtil::scale(s._transform.toMatrix(), toScale(s._size, metrics)));
                const ByteArray::Borrowed divided = s._varyings.getDivided(1);
                if(divided.length() > sizeof(M4))
                    writer.write(divided.buf() + sizeof(M4), divided.length() - sizeof(M4), sizeof(M4));
            }
            ++ offset;
        }
}

V3 RCCMultiDrawElementsIndirect::toScale(const V3& displaySize, const Metrics& metrics) const
{
    const V3& size = metrics.size;
    return V3(displaySize.x() != 0 ? displaySize.x() / size.x() : 1.0f,
              displaySize.y() != 0 ? displaySize.y() / size.y() : 1.0f,
              displaySize.z() != 0 ? displaySize.z() /  size.z() : 1.0f);
}

RCCMultiDrawElementsIndirect::VerticesUploader::VerticesUploader(const sp<ModelBundle>& multiModels, const sp<PipelineInput>& pipelineInput)
    : Uploader(multiModels->vertexLength() * pipelineInput->getStream(0).stride()), _model_bundle(multiModels), _pipeline_input(pipelineInput)
{
}

void RCCMultiDrawElementsIndirect::VerticesUploader::upload(Writable& uploader)
{
    uint32_t offset = 0;
    size_t stride = _pipeline_input->getStream(0).stride();
    PipelineInput::AttributeOffsets attributes(_pipeline_input);
    for(const auto& i : _model_bundle->models())
    {
        const Model& model = i.second._model;
        uint32_t size = static_cast<uint32_t>(model.vertices()->length() * stride);
        std::vector<uint8_t> buf(size);
        VertexWriter stream(attributes, false, buf.data(), size, stride);
        i.second._model->writeToStream(stream, V3(1.0f));
        uploader.write(buf.data(), size, offset);
        offset += size;
    }
}

RCCMultiDrawElementsIndirect::IndicesUploader::IndicesUploader(const sp<ModelBundle>& multiModels)
    : Uploader(multiModels->indexLength() * sizeof(element_index_t)), _model_bundle(multiModels)
{
}

namespace {

class WritableWithOffset : public Writable {
public:
    WritableWithOffset(Writable& delegate, uint32_t offset)
        : _delegate(delegate), _offset(offset) {
    }

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
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
    for(const auto& i: _model_bundle->models())
    {
        const sp<Uploader>& indices = i.second._model->indices();
        uint32_t size = static_cast<uint32_t>(indices->size());
        WritableWithOffset writable(uploader, offset);
        indices->upload(writable);
        offset += size;
    }
}

}
