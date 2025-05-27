#include "renderer/impl/render_command_composer/rcc_multi_draw_elements_indirect.h"

#include "core/base/named_hash.h"
#include "core/impl/writable/writable_with_offset.h"
#include "graphics/base/material.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/impl/mat/mat4_impl.h"
#include "graphics/util/matrix_util.h"
#include "graphics/util/mat4_type.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/node.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"


namespace ark {

namespace {

class VerticesUploader final : public Uploader {
public:
    VerticesUploader(sp<ModelBundle> multiModels, sp<PipelineLayout> pipelineInput)
        : Uploader(multiModels->vertexLength() * pipelineInput->getStreamLayout(0).stride()), _model_bundle(std::move(multiModels)), _pipeline_input(std::move(pipelineInput)) {
    }

    void upload(Writable& uploader) override {
        uint32_t offset = 0;
        const size_t stride = _pipeline_input->getStreamLayout(0).stride();
        PipelineLayout::VertexDescriptor attributes(_pipeline_input);
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
    sp<PipelineLayout> _pipeline_input;

};

class IndicesUploader final : public Uploader {
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

V3 toScale(const V3& displaySize, const Boundaries& metrics)
{
    const V3& size = metrics.size()->val();
    return {displaySize.x() != 0 ? displaySize.x() / size.x() : 1.0f,
              displaySize.y() != 0 ? displaySize.y() / size.y() : 1.0f,
              displaySize.z() != 0 ? displaySize.z() /  size.z() : 1.0f};
}

}

RCCMultiDrawElementsIndirect::RCCMultiDrawElementsIndirect(sp<ModelBundle> multiModels)
    : _model_bundle(std::move(multiModels))
{
}

sp<PipelineBindings> RCCMultiDrawElementsIndirect::makePipelineBindings(const Shader& shader, RenderController& renderController, enums::DrawMode renderMode)
{
    _indices = renderController.makeIndexBuffer({}, sp<IndicesUploader>::make(_model_bundle));
    _draw_indirect = renderController.makeBuffer({Buffer::USAGE_BIT_DRAW_INDIRECT, Buffer::USAGE_BIT_DYNAMIC}, nullptr);
    return shader.makeBindings(renderController.makeVertexBuffer({}, sp<VerticesUploader>::make(_model_bundle, shader.layout())), renderMode, enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT);
}

DrawingContext RCCMultiDrawElementsIndirect::compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot)
{
    DrawingBuffer buf(snapshot._stub->_pipeline_bindings, snapshot._stub->_stride);
    const Buffer& vertices = snapshot._stub->_pipeline_bindings->vertices();
    const bool reload = snapshot.needsReload();

    if(reload)
        reloadIndirectCommands(snapshot);

    writeModelMatices(renderRequest, buf, snapshot, reload);

    DrawingParams::DrawMultiElementsIndirect drawParams{buf.toDividedBufferSnapshots(), reload ? _draw_indirect.snapshot(makeIndirectBuffer(renderRequest)) : _draw_indirect.snapshot(), static_cast<uint32_t>(_indirect_cmds.size())};
    return snapshot.toDrawingContext(renderRequest, vertices.snapshot(), _indices.snapshot(), 0, std::move(drawParams));
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

void RCCMultiDrawElementsIndirect::writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayerSnapshot& renderLayerSnapshot, bool reload)
{
    const auto& renderLayerItems = renderLayerSnapshot._elements;
    for(size_t i = 0; i < renderLayerItems.size(); ++i)
    {
        const RenderLayerSnapshot::Element& s = renderLayerItems.at(i);
        const Renderable::Snapshot& snapshot = s._snapshot;
        if(reload || s._snapshot._state.has(Renderable::RENDERABLE_STATE_DIRTY))
            if(!snapshot._varyings_snapshot._sub_properties.empty())
            {
                const auto iter = _model_instances.find(i);
                DASSERT(iter != _model_instances.end());

                if(!iter->second.isDynamicLayout())
                    iter->second.toDynamicLayout();

                for(const auto& [j, k] : snapshot._varyings_snapshot._sub_properties)
                    if(Varyings::Divided subProperty = k.getDivided(1); subProperty._content.length() > sizeof(M4))
                        iter->second.setNodeTransform(j, *reinterpret_cast<const M4*>(subProperty._content.buf()));
            }
    }

    size_t instanceId = 0;
    const PipelineLayout::VertexDescriptor& attributeOffsets = buf.pipelineBindings()->pipelineDescriptor()->vertexDescriptor();
    const size_t attributeStride = attributeOffsets._stride;
    const bool hasModelMatrix = attributeOffsets._offsets[Attribute::USAGE_MODEL_MATRIX] != -1;
    const bool hasMaterialId = attributeOffsets._offsets[Attribute::USAGE_MATERIAL_ID] != -1;
    for(const IndirectCmds& i : _indirect_cmds.values())
        for(const auto& [nodeInstance, mesh] : i._mesh_instances)
        {
            const RenderLayerSnapshot::Element& s = renderLayerItems.at(nodeInstance->snapshotIndex());
            const Renderable::Snapshot& snapshot = s._snapshot;
            if(reload || snapshot._state.has(Renderable::RENDERABLE_STATE_DIRTY))
                if(snapshot._varyings_snapshot._buffers.length() > 0)
                {
                    const ModelBundle::ModelLayout& modelLayout = _model_bundle->ensureModelLayout(snapshot._type);
                    const Boundaries& metrics = modelLayout._model->content();
                    VertexWriter writer = buf.makeDividedVertexWriter(renderRequest, 1, instanceId, 1);
                    writer.next();
                    if(hasModelMatrix)
                    {
                        const bool visible = snapshot._state.has(Renderable::RENDERABLE_STATE_VISIBLE);
                        const M4 modelMatrix = visible ? MatrixUtil::translate({}, snapshot._position) * MatrixUtil::scale(snapshot._transform->toMatrix(snapshot._transform_snapshot), toScale(snapshot._size, metrics)) * nodeInstance->globalTransform()
                                                       : MatrixUtil::scale(M4(), V3(0));
                        writer.writeAttribute(modelMatrix, Attribute::USAGE_MODEL_MATRIX);
                    }
                    if(hasMaterialId && mesh->material())
                        writer.writeAttribute(mesh->material()->id(), Attribute::USAGE_MATERIAL_ID);
                    if(ByteArray::Borrowed divided = snapshot._varyings_snapshot.getDivided(1)._content; divided.length() > attributeStride)
                        writer.write(divided.buf() + attributeStride, divided.length() - attributeStride, attributeStride);
                }
            ++ instanceId;
        }
}

void RCCMultiDrawElementsIndirect::reloadIndirectCommands(const RenderLayerSnapshot& snapshot)
{
    size_t offset = 0;
    _indirect_cmds.clear();
    _model_instances.clear();
    for(const RenderLayerSnapshot::Element& i : snapshot._elements)
    {
        const int32_t type = i._snapshot._type;
        const ModelBundle::ModelLayout& modelLayout = _model_bundle->ensureModelLayout(type);
        ModelInstance& modelInstance = _model_instances[offset];
        modelInstance = ModelInstance(offset, modelLayout);
        for(const ModelBundle::NodeLayout& j : modelLayout._node_layouts)
        {
            sp<NodeInstance> nodeInstance = sp<NodeInstance>::make(modelInstance, j._node->name().hash());
            for(const sp<Mesh>& k : j._node->meshes())
            {
                uint64_t cmdHash = 0;
                Math::hashCombine(cmdHash, type);
                Math::hashCombine(cmdHash, k->id());
                const ModelBundle::MeshLayout& meshLayout = modelLayout._mesh_layouts.at(k->id());
                IndirectCmds& modelIndirect = _indirect_cmds[cmdHash];
                if(modelIndirect._mesh_instances.empty())
                    modelIndirect._command = {static_cast<uint32_t>(meshLayout._mesh->indices().size()), 0, static_cast<uint32_t>(meshLayout._index_offset), static_cast<uint32_t>(meshLayout._vertex_offset), 0};

                ++ modelIndirect._command._instance_count;
                modelIndirect._mesh_instances.push_back(MeshInstance{nodeInstance, meshLayout._mesh});
            }
        }
        offset ++;
    }
}

RCCMultiDrawElementsIndirect::NodeLayoutInstance::NodeLayoutInstance(const Node& node, const NodeLayoutInstance& parentLayout)
    : _node_id(node.name().hash()), _node_transform(sp<Mat4::Impl>::make(M4::identity())), _global_transform(makeGlobalTransform(parentLayout._global_transform, node.localMatrix()))
{
}

RCCMultiDrawElementsIndirect::NodeLayoutInstance::NodeLayoutInstance(const Node& node, const M4& globalTransform)
    : _node_id(node.name().hash()), _global_transform(sp<Mat4>::make<Mat4Impl>(globalTransform))
{
}

sp<Mat4> RCCMultiDrawElementsIndirect::NodeLayoutInstance::makeGlobalTransform(sp<Mat4> parentTransform, const M4& localTransform) const
{
    sp<Mat4> nodeTransform = Mat4Type::matmul(sp<Mat4>::make<Mat4::Const>(localTransform), static_cast<sp<Mat4>>(_node_transform));
    return parentTransform ? Mat4Type::matmul(std::move(parentTransform), std::move(nodeTransform)) : nodeTransform;
}

RCCMultiDrawElementsIndirect::ModelInstance::ModelInstance(const size_t snapshotIndex, const ModelBundle::ModelLayout& modelLayout)
    : _model(modelLayout._model), _snapshot_index(snapshotIndex)
{
    for(const ModelBundle::NodeLayout& i : modelLayout._node_layouts)
    {
        NodeLayoutInstance nodeLayout(i._node, i._transform);
        HashId nodeId = nodeLayout._node_id;
        ASSERT(_node_layout_instances.find(nodeId) == _node_layout_instances.end());
        _node_layout_instances.insert(std::make_pair(nodeId, std::move(nodeLayout)));
    }
}

bool RCCMultiDrawElementsIndirect::ModelInstance::isDynamicLayout() const
{
    if(_node_layout_instances.empty())
        return true;

    return static_cast<bool>(_node_layout_instances.begin()->second._node_transform);
}

void RCCMultiDrawElementsIndirect::ModelInstance::toDynamicLayout()
{
    for(NodeLayoutInstance& i : _model->toFlatLayouts<NodeLayoutInstance>())
        _node_layout_instances[i._node_id] = std::move(i);
}

void RCCMultiDrawElementsIndirect::ModelInstance::setNodeTransform(const size_t nodeId, const M4& transform)
{
    const auto iter = _node_layout_instances.find(nodeId);
    CHECK(iter != _node_layout_instances.end(), "Node(%s) does not exist", NamedHash::reverse(nodeId).c_str());
    ASSERT(iter->second._node_transform);
    iter->second._node_transform->set(transform);
}

RCCMultiDrawElementsIndirect::NodeInstance::NodeInstance(ModelInstance& modelInstance, const HashId nodeId)
    : _model_instance(modelInstance), _node_id(nodeId)
{
}

M4 RCCMultiDrawElementsIndirect::NodeInstance::globalTransform() const
{
    const auto iter = _model_instance._node_layout_instances.find(_node_id);
    DASSERT(iter != _model_instance._node_layout_instances.end());
    return iter->second._global_transform->val();
}

size_t RCCMultiDrawElementsIndirect::NodeInstance::snapshotIndex() const
{
    return _model_instance._snapshot_index;
}

}
