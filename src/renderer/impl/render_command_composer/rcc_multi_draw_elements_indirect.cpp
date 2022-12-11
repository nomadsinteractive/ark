#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "core/impl/writable/writable_with_offset.h"

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

namespace {

class VerticesUploader : public Input {
public:
    VerticesUploader(sp<ModelBundle> multiModels, sp<PipelineInput> pipelineInput)
        : Input(multiModels->vertexLength() * pipelineInput->getStream(0).stride()), _model_bundle(std::move(multiModels)), _pipeline_input(std::move(pipelineInput)) {
    }

    void upload(Writable& uploader) override {
        uint32_t offset = 0;
        size_t stride = _pipeline_input->getStream(0).stride();
        PipelineInput::AttributeOffsets attributes(_pipeline_input);
        for(const ModelBundle::ModelLayout& i : _model_bundle->modelLayouts().values())
        {
            const Model& model = i._model;
            uint32_t size = static_cast<uint32_t>(model.vertexCount() * stride);
            std::vector<uint8_t> buf(size);
            VertexWriter stream(attributes, false, buf.data(), size, stride);
            model.writeToStream(stream, V3(1.0f));
            uploader.write(buf.data(), size, offset);
            offset += size;
        }
    }

private:
    sp<ModelBundle> _model_bundle;
    sp<PipelineInput> _pipeline_input;
};

class IndicesUploader : public Input {
public:
    IndicesUploader(sp<ModelBundle> multiModels)
        : Input(multiModels->indexLength() * sizeof(element_index_t)), _model_bundle(std::move(multiModels)) {
    }

    void upload(Writable& uploader) override {
        size_t offset = 0;
        for(const auto& i: _model_bundle->modelLayouts())
        {
            Input& indices = i.second._model->indices();
            indices.upload(WritableWithOffset(uploader, offset));
            offset += indices.size();
        }
    }

private:
    sp<ModelBundle> _model_bundle;
};

}

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
        size_t offset = 0;
        _indirect_cmds.clear();
        for(const Renderable::Snapshot& i : snapshot._items)
        {
            const ModelBundle::ModelLayout& modelInfo = _model_bundle->ensureModelInfo(i._type);
            IndirectCmds& modelIndirect = _indirect_cmds[static_cast<uint64_t>(i._type) << 32];
            if(modelIndirect._snapshot_indices.empty())
                modelIndirect._command = {static_cast<uint32_t>(modelInfo._model->indexCount()), 0, static_cast<uint32_t>(modelInfo._index_offset), static_cast<uint32_t>(modelInfo._vertex_offset), 0};

            ++ modelIndirect._command._instance_count;
            modelIndirect._snapshot_indices.push_back(offset);
            offset ++;
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
    for(const IndirectCmds& i : _indirect_cmds.values())
    {
        *pcmds = i._command;
        pcmds->_base_instance = baseInstance;
        baseInstance += pcmds->_instance_count;
        ++ pcmds;
    }
    return cmds;
}

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot, bool reload)
{
    size_t offset = 0;
    for(const IndirectCmds& i : _indirect_cmds.values())
        for(size_t j : i._snapshot_indices)
        {
            const Renderable::Snapshot& s = snapshot._items.at(j);
            if(reload || s.getState(Renderable::RENDERABLE_STATE_DIRTY))
            {
                const ModelBundle::ModelLayout& modelInfo = _model_bundle->ensureModelInfo(s._type);
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
    const V3& size = metrics.size();
    return V3(displaySize.x() != 0 ? displaySize.x() / size.x() : 1.0f,
              displaySize.y() != 0 ? displaySize.y() / size.y() : 1.0f,
              displaySize.z() != 0 ? displaySize.z() /  size.z() : 1.0f);
}

}
