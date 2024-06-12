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

class VerticesUploader : public Uploader {
public:
    VerticesUploader(sp<ModelBundle> multiModels, sp<PipelineInput> pipelineInput)
        : Uploader(multiModels->vertexLength() * pipelineInput->getStream(0).stride()), _model_bundle(std::move(multiModels)), _pipeline_input(std::move(pipelineInput)) {
    }

    void upload(Writable& uploader) override {
        uint32_t offset = 0;
        size_t stride = _pipeline_input->getStream(0).stride();
        PipelineInput::AttributeOffsets attributes(_pipeline_input);
        for(const ModelBundle::ModelLayout& i : _model_bundle->modelLayouts().values())
        {
            const Model& model = i._model;
            const uint32_t size = static_cast<uint32_t>(model.vertexCount() * stride);
            std::vector<uint8_t> buf(size);
            VertexWriter stream(attributes, false, buf.data(), size, stride);
            model.writeToStream(stream, V3(1.0f));
            uploader.write(buf.data(), size, offset);
            offset += size;
        }
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    sp<ModelBundle> _model_bundle;
    sp<PipelineInput> _pipeline_input;

};

class IndicesUploader : public Uploader {
public:
    IndicesUploader(sp<ModelBundle> multiModels)
        : Uploader(multiModels->indexLength() * sizeof(element_index_t)), _model_bundle(std::move(multiModels)) {
    }

    void upload(Writable& uploader) override {
        size_t offset = 0;
        for(const auto& i: _model_bundle->modelLayouts())
        {
            Uploader& indices = i.second._model->indices();
            WritableWithOffset writable(uploader, offset);
            indices.upload(writable);
            offset += indices.size();
        }
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    sp<ModelBundle> _model_bundle;

};

}

RCCMultiDrawElementsIndirect::RCCMultiDrawElementsIndirect(sp<ModelBundle> multiModels)
    : _model_bundle(std::move(multiModels))
{
}

sp<ShaderBindings> RCCMultiDrawElementsIndirect::makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode)
{
    _indices = renderController.makeIndexBuffer(Buffer::USAGE_STATIC, sp<IndicesUploader>::make(_model_bundle));
    _draw_indirect = renderController.makeBuffer(Buffer::TYPE_DRAW_INDIRECT, Buffer::USAGE_DYNAMIC, nullptr);
    return shader.makeBindings(renderController.makeVertexBuffer(Buffer::USAGE_STATIC, sp<VerticesUploader>::make(_model_bundle, shader.input())), renderMode, Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT);
}

void RCCMultiDrawElementsIndirect::postSnapshot(RenderController& /*renderController*/, RenderLayerSnapshot& /*snapshot*/)
{
}

sp<RenderCommand> RCCMultiDrawElementsIndirect::compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot)
{
    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    const Buffer& vertices = snapshot._stub->_shader_bindings->vertices();
    bool reload = snapshot.needsReload();

    if(reload)
        reloadIndirectCommands(snapshot);

    writeModelMatices(renderRequest, buf, snapshot, reload);

    DrawingParams::DrawMultiElementsIndirect drawParams{buf.toDividedBufferSnapshots(), reload ? _draw_indirect.snapshot(makeIndirectBuffer(renderRequest)) : _draw_indirect.snapshot(), static_cast<uint32_t>(_indirect_cmds.size())};
    return snapshot.toRenderCommand(renderRequest, vertices.snapshot(), _indices.snapshot(), 0, std::move(drawParams));
}

ByteArray::Borrowed RCCMultiDrawElementsIndirect::makeIndirectBuffer(const RenderRequest& renderRequest) const
{
    ByteArray::Borrowed cmds = renderRequest.allocator().sbrkSpan(_indirect_cmds.size() * sizeof(DrawingParams::DrawElementsIndirectCommand));
    DrawingParams::DrawElementsIndirectCommand* pcmds = reinterpret_cast<DrawingParams::DrawElementsIndirectCommand*>(cmds.buf());
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

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, RenderLayerSnapshot& renderLayerSnapshot, bool reload)
{
    auto& renderLayerItems = renderLayerSnapshot._droplets;
    for(size_t i = 0; i < renderLayerItems.size(); ++i)
    {
        RenderLayerSnapshot::Droplet& s = renderLayerItems.at(i);
        const Renderable::Snapshot& snapshot = reload ? s.ensureDirtySnapshot(renderRequest) : s._snapshot;
        if(reload || s._snapshot._state.hasState(Renderable::RENDERABLE_STATE_DIRTY))
        {
            if(snapshot._varyings._sub_properties.size() > 0)
            {
                const auto iter = _model_instances.find(i);
                DASSERT(iter != _model_instances.end());

                if(!iter->second.isDynamicLayout())
                    iter->second.toDynamicLayout();

                for(const auto& [j, k] : snapshot._varyings._sub_properties)
                {
                    Varyings::Divided subProperty = k.getDivided(1);
                    if(subProperty._content.length() > sizeof(M4))
                        iter->second.setNodeTransform(j, *reinterpret_cast<const M4*>(subProperty._content.buf()));
                }
            }
        }
    }

    size_t offset = 0;
    for(const IndirectCmds& i : _indirect_cmds.values())
        for(const NodeInstance& j : i._node_instances)
        {
            const RenderLayerSnapshot::Droplet& s = renderLayerItems.at(j.snapshotIndex());
            const Renderable::Snapshot& snapshot = s._snapshot;
            if(reload || snapshot._state.hasState(Renderable::RENDERABLE_STATE_DIRTY))
                if(snapshot._varyings._buffers.length() > 0)
                {
                    const ModelBundle::ModelLayout& modelInfo = _model_bundle->ensureModelLayout(snapshot._type);
                    const Boundaries& metrics = modelInfo._model->content();
                    VertexWriter writer = buf.makeDividedVertexWriter(renderRequest, 1, offset, 1);
                    writer.next();
                    writer.write(MatrixUtil::translate(M4::identity(), snapshot._position) * MatrixUtil::scale(snapshot._transform.toMatrix(), toScale(snapshot._size, metrics)) * j.globalTransform());
                    ByteArray::Borrowed divided = snapshot._varyings.getDivided(1)._content;
                    if(divided.length() > sizeof(M4))
                        writer.write(divided.buf() + sizeof(M4), divided.length() - sizeof(M4), sizeof(M4));
                }
            ++ offset;
        }
}

V3 RCCMultiDrawElementsIndirect::toScale(const V3& displaySize, const Boundaries& metrics) const
{
    const V3& size = metrics.size()->val();
    return V3(displaySize.x() != 0 ? displaySize.x() / size.x() : 1.0f,
              displaySize.y() != 0 ? displaySize.y() / size.y() : 1.0f,
              displaySize.z() != 0 ? displaySize.z() /  size.z() : 1.0f);
}

void RCCMultiDrawElementsIndirect::reloadIndirectCommands(const RenderLayerSnapshot& snapshot)
{
    size_t offset = 0;
    _indirect_cmds.clear();
    _model_instances.clear();
    for(const RenderLayerSnapshot::Droplet& i : snapshot._droplets)
    {
        int32_t type = i._snapshot._type;
        const ModelBundle::ModelLayout& modelLayout = _model_bundle->ensureModelLayout(type);
        ModelInstance& modelInstance = _model_instances[offset];
        modelInstance = ModelInstance(offset, modelLayout);
        for(const ModelBundle::NodeLayout& j : modelLayout._node_layouts)
        {
            sp<NodeInstance> nodeInstance = sp<NodeInstance>::make(modelInstance, j._node->name().hash());
            for(const sp<Mesh>& k : j._node->meshes())
            {
                const ModelBundle::MeshLayout& meshLayout = modelLayout._mesh_layouts.at(k->id());
                IndirectCmds& modelIndirect = _indirect_cmds[static_cast<uint64_t>(type) << 32 | k->id()];
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
    : _node_id(node.name().hash()), _node_transform(sp<Mat4::Impl>::make(M4::identity())), _global_transform(makeGlobalTransform(parentLayout._global_transform, node.transform()))
{
}

sp<Mat4> RCCMultiDrawElementsIndirect::NodeLayoutInstance::makeGlobalTransform(sp<Mat4> parentTransform, const M4& localTransform) const
{
    sp<Mat4> nodeTransform = Mat4Type::matmul(sp<Mat4>::make<Mat4::Const>(localTransform), static_cast<sp<Mat4>>(_node_transform));
    return parentTransform ? Mat4Type::matmul(std::move(parentTransform), std::move(nodeTransform)) : nodeTransform;
}

RCCMultiDrawElementsIndirect::NodeLayoutInstance::NodeLayoutInstance(const Node& node, const M4& globalTransform)
    : _node_id(node.name().hash()), _global_transform(sp<Mat4Impl>::make(globalTransform))
{
}

RCCMultiDrawElementsIndirect::ModelInstance::ModelInstance(size_t snapshotIndex, const ModelBundle::ModelLayout& modelLayout)
    : _model(modelLayout._model), _snapshot_index(snapshotIndex)
{
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
