#pragma once

#include <mutex>
#include <vector>
#include <unordered_map>

#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_bundle.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/inf/drawing_context_composer.h"


namespace ark {

class ARK_API RCCMultiDrawElementsIndirect final : public DrawingContextComposer {
public:
    RCCMultiDrawElementsIndirect(sp<ModelBundle> multiModels);

    sp<PipelineBindings> makePipelineBindings(const Shader& shader, RenderController& renderController, Enum::RenderMode renderMode) override;
    DrawingContext compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot) override;

private:
    struct NodeLayoutInstance {
        NodeLayoutInstance() = default;
        NodeLayoutInstance(const Node& node, const M4& globalTransform);
        NodeLayoutInstance(const Node& node, const NodeLayoutInstance& parentLayout);

        HashId _node_id;

        sp<Mat4::Impl> _node_transform;
        sp<Mat4> _global_transform;

    private:
        sp<Mat4> makeGlobalTransform(sp<Mat4> parentTransform, const M4& localTransform) const;
    };

    struct ModelInstance;

    struct NodeInstance {
        NodeInstance(ModelInstance& modelInstance, HashId nodeId);

        M4 globalTransform() const;
        size_t snapshotIndex() const;

        ModelInstance& _model_instance;
        HashId _node_id;
    };

    struct MeshInstance {
        sp<NodeInstance> _node_instance;
        sp<Mesh> _mesh;
    };

    struct ModelInstance {
        ModelInstance() = default;
        ModelInstance(size_t snapshotIndex, const ModelBundle::ModelLayout& modelLayout);

        bool isDynamicLayout() const;
        void toDynamicLayout();

        void setNodeTransform(size_t nodeId, const M4& transform);

        sp<Model> _model;
        size_t _snapshot_index;
        std::map<HashId, NodeLayoutInstance> _node_layout_instances;
    };

    struct IndirectCmds {
        DrawingParams::DrawElementsIndirectCommand _command;
        std::vector<MeshInstance> _mesh_instances;
    };

private:
    ByteArray::Borrowed makeIndirectBuffer(const RenderRequest& renderRequest) const;
    void writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayerSnapshot& renderLayerSnapshot, bool reload);

    void reloadIndirectCommands(const RenderLayerSnapshot& snapshot);

private:
    sp<ModelBundle> _model_bundle;

    Buffer _indices;
    Buffer _draw_indirect;

    Table<uint64_t, IndirectCmds> _indirect_cmds;
    std::unordered_map<size_t, ModelInstance> _model_instances;
};

}
