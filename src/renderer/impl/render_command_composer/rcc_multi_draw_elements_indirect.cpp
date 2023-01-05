#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "core/impl/writable/writable_with_offset.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/impl/mat/mat4_impl.h"
#include "graphics/util/matrix_util.h"
#include "graphics/util/mat4_type.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/node.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"


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
            WritableWithOffset writable(uploader, offset);
            indices.upload(writable);
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

void RCCMultiDrawElementsIndirect::postSnapshot(RenderController& /*renderController*/, RenderLayerSnapshot& /*snapshot*/)
{
}

sp<RenderCommand> RCCMultiDrawElementsIndirect::compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot)
{
    const std::lock_guard<std::mutex> lg(_mutex);
    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    const Buffer& vertices = snapshot._stub->_shader_bindings->vertices();
    bool reload = snapshot.needsReload();

    if(reload)
        reloadIndirectCommands(snapshot);

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

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayerSnapshot& snapshot, bool reload)
{
    for(size_t i = 0; i < snapshot._items.size(); ++i)
    {
        const Renderable::Snapshot& s = snapshot._items.at(i);
        if(reload || s.getState(Renderable::RENDERABLE_STATE_DIRTY))
        {
            if(s._varyings._sub_properties.size() > 0)
            {
                const auto iter = _model_instances.find(i);
                DASSERT(iter != _model_instances.end());

                if(!iter->second.isDynamicLayout())
                    iter->second.toDynamicLayout();

                for(const auto& [j, k] : s._varyings._sub_properties)
                {
                    const ByteArray::Borrowed subProperty = k.getDivided(1);
                    if(subProperty.length() > sizeof(M4))
                        iter->second.setNodeTransform(j, *reinterpret_cast<const M4*>(subProperty.buf()));
                }
            }
        }
    }

    size_t offset = 0;
    for(const IndirectCmds& i : _indirect_cmds.values())
        for(const NodeInstance& j : i._node_instances)
        {
            const Renderable::Snapshot& s = snapshot._items.at(j.snapshotIndex());
            if(reload || s.getState(Renderable::RENDERABLE_STATE_DIRTY))
            {
                const ModelBundle::ModelLayout& modelInfo = _model_bundle->ensureModelLayout(s._type);
                const Metrics& metrics = modelInfo._model->metrics();
                VertexWriter writer = buf.makeDividedVertexWriter(renderRequest, 1, offset, 1);
                writer.next();
                writer.write(MatrixUtil::translate(M4::identity(), s._position) * MatrixUtil::scale(s._transform.toMatrix(), toScale(s._size, metrics)) * j.globalTransform());
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

void RCCMultiDrawElementsIndirect::reloadIndirectCommands(const RenderLayerSnapshot& snapshot)
{
    size_t offset = 0;
    _indirect_cmds.clear();
    _model_instances.clear();
    for(const Renderable::Snapshot& i : snapshot._items)
    {
        const ModelBundle::ModelLayout& modelLayout = _model_bundle->ensureModelLayout(i._type);
        ModelInstance& modelInstance = _model_instances[offset];
        modelInstance = ModelInstance(offset, modelLayout);
        for(const ModelBundle::NodeLayout& j : modelLayout._node_layouts)
        {
            sp<NodeInstance> nodeInstance = sp<NodeInstance>::make(modelInstance, j._node->name().hash());
            for(const sp<Mesh>& k : j._node->meshes())
            {
                const ModelBundle::MeshLayout& meshLayout = modelLayout._mesh_layouts.at(k->id());
                IndirectCmds& modelIndirect = _indirect_cmds[static_cast<uint64_t>(i._type) << 32 | k->id()];
                if(modelIndirect._node_instances.empty())
                    modelIndirect._command = {static_cast<uint32_t>(meshLayout._mesh->indices().size()), 0, static_cast<uint32_t>(meshLayout._index_offset), static_cast<uint32_t>(modelLayout._vertex_offset), 0};

                ++ modelIndirect._command._instance_count;
                modelIndirect._node_instances.push_back(nodeInstance);
            }
        }
        offset ++;
    }
}

RCCMultiDrawElementsIndirect::NodeLayoutInstance::NodeLayoutInstance(const Node& node, const NodeLayoutInstance& parentLayout)
    : _node_id(node.name().hash()), _node_transform(sp<Mat4Impl>::make(node.transform())), _global_transform(parentLayout._global_transform ? Mat4Type::matmul(parentLayout._global_transform, static_cast<sp<Mat4>>(_node_transform)) : static_cast<sp<Mat4>>(_node_transform)) {
}

RCCMultiDrawElementsIndirect::NodeLayoutInstance::NodeLayoutInstance(const Node& node, const M4& globalTransform)
    : _node_id(node.name().hash()), _global_transform(sp<Mat4Impl>::make(globalTransform)) {
}

RCCMultiDrawElementsIndirect::ModelInstance::ModelInstance(size_t snapshotIndex, const ModelBundle::ModelLayout& modelLayout)
    : _model(modelLayout._model), _snapshot_index(snapshotIndex) {
    for(const ModelBundle::NodeLayout& i : modelLayout._node_layouts)
    {
        NodeLayoutInstance nodeLayout(i._node, i._transform);
        size_t nodeId = nodeLayout._node_id;
        _node_layouts.insert(std::make_pair(nodeId, std::move(nodeLayout)));
    }
}

bool RCCMultiDrawElementsIndirect::ModelInstance::isDynamicLayout() const
{
    if(_node_layouts.empty())
        return true;

    return static_cast<bool>(_node_layouts.begin()->second._node_transform);
}

void RCCMultiDrawElementsIndirect::ModelInstance::toDynamicLayout()
{
    for(NodeLayoutInstance& i : _model->toFlatLayouts<NodeLayoutInstance>())
        _node_layouts[i._node_id] = std::move(i);
}

void RCCMultiDrawElementsIndirect::ModelInstance::setNodeTransform(size_t nodeId, const M4& transform)
{
    const auto iter = _node_layouts.find(nodeId);
    DASSERT(iter != _node_layouts.end());
    DASSERT(iter->second._node_transform);
    iter->second._node_transform->set(transform);
}

RCCMultiDrawElementsIndirect::NodeInstance::NodeInstance(ModelInstance& modelInstance, size_t nodeId)
    : _model_instance(modelInstance), _node_id(nodeId)
{
}

M4 RCCMultiDrawElementsIndirect::NodeInstance::globalTransform() const
{
    const auto iter = _model_instance._node_layouts.find(_node_id);
    DASSERT(iter != _model_instance._node_layouts.end());
    return iter->second._global_transform->val();
}

size_t RCCMultiDrawElementsIndirect::NodeInstance::snapshotIndex() const
{
    return _model_instance._snapshot_index;
}

}
