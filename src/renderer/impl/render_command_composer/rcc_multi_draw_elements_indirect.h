#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_MULTI_DRAW_ELEMENTS_INDIRECT_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_MULTI_DRAW_ELEMENTS_INDIRECT_H_

#include <mutex>
#include <vector>
#include <unordered_map>

#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_bundle.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"


namespace ark {

class ARK_API RCCMultiDrawElementsIndirect : public RenderCommandComposer {
public:
    RCCMultiDrawElementsIndirect(sp<ModelBundle> multiModels);

    virtual sp<ShaderBindings> makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot) override;
    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot) override;

private:
    struct NodeLayoutInstance {
        NodeLayoutInstance() = default;
        NodeLayoutInstance(const Node& node, const M4& globalTransform);
        NodeLayoutInstance(const Node& node, const NodeLayoutInstance& parentLayout);

        size_t _node_id;

        sp<Mat4::Impl> _node_transform;
        sp<Mat4> _global_transform;

    private:
        sp<Mat4> makeGlobalTransform(sp<Mat4> parentTransform, const M4& localTransform) const;
    };

    struct ModelInstance;

    struct NodeInstance {
        NodeInstance(ModelInstance& modelInstance, size_t nodeId);

        M4 globalTransform() const;
        size_t snapshotIndex() const;

        ModelInstance& _model_instance;
        size_t _node_id;
    };

    struct ModelInstance {
        ModelInstance() = default;
        ModelInstance(size_t snapshotIndex, const ModelBundle::ModelLayout& modelLayout);

        bool isDynamicLayout() const;
        void toDynamicLayout();

        void setNodeTransform(size_t nodeId, const M4& transform);

        sp<Model> _model;
        size_t _snapshot_index;
        std::map<size_t, NodeLayoutInstance> _node_layouts;
    };

    struct IndirectCmds {
        DrawingContext::DrawElementsIndirectCommand _command;
        std::vector<sp<NodeInstance>> _node_instances;
    };

private:
    ByteArray::Borrowed makeIndirectBuffer(const RenderRequest& renderRequest) const;
    void writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayerSnapshot& snapshot, bool reload);
    V3 toScale(const V3& size, const Metrics& metrics) const;

    void reloadIndirectCommands(const RenderLayerSnapshot& snapshot);

private:
//TODO: make RenderCommandComposer thread safe.
[[deprecated]]
    std::mutex _mutex;

    sp<ModelBundle> _model_bundle;

    Buffer _indices;
    Buffer _draw_indirect;

    Table<uint64_t, IndirectCmds> _indirect_cmds;
    std::unordered_map<size_t, ModelInstance> _model_instances;
};

}

#endif
