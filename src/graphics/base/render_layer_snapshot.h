#pragma once

#include "core/base/api.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/layer_context_snapshot.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/rect.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/drawing_params.h"

namespace ark {

class RenderLayerSnapshot {
public:
    struct UBOSnapshot {
        ByteArray::Borrowed _dirty_flags;
        ByteArray::Borrowed _buffer;
    };

    enum SnapshotFlag {
        SNAPSHOT_FLAG_RELOAD,
        SNAPSHOT_FLAG_DYNAMIC_UPDATE,
        SNAPSHOT_FLAG_STATIC_MODIFIED,
        SNAPSHOT_FLAG_STATIC_REUSE
    };

    struct Droplet {
        Droplet(Renderable& renderable, const LayerContextSnapshot& layerContext, LayerContext::ElementState& state, const Renderable::Snapshot& snapshot);

        const Renderable::Snapshot& ensureDirtySnapshot(const RenderRequest& renderRequest);

        Renderable& _renderable;
        const LayerContextSnapshot& _layer_context;
        LayerContext::ElementState& _element_state;
        Renderable::Snapshot _snapshot;
    };

public:
    RenderLayerSnapshot(RenderLayerSnapshot&& other) = default;

    sp<RenderCommand> compose(const RenderRequest& renderRequest);

    bool needsReload() const;
    const sp<PipelineInput>& pipelineInput() const;

    void snapshot(RenderRequest& renderRequest, std::vector<sp<LayerContext>>& layerContexts);

    bool addDisposedState(LayerContext& lc, void* stateKey);

    void addDiscardedLayerContext(LayerContext& lc);
    void addDiscardedLayerContexts(const std::vector<sp<LayerContext>>& layerContexts);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams params);

    sp<RenderLayer::Stub> _stub;

    size_t _index_count;

    std::vector<UBOSnapshot> _ubos;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;

    std::vector<LayerContextSnapshot> _layer_context_snapshots;

    std::deque<Droplet> _droplets;
    std::deque<LayerContext::ElementState> _item_deleted;
    Rect _scissor;
    bool _needs_reload;

    DISALLOW_COPY_AND_ASSIGN(RenderLayerSnapshot);

private:
    bool addLayerContext(RenderRequest& renderRequest, LayerContext& layerContext);

private:

    RenderLayerSnapshot(RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub);

    friend class RenderLayer;
};

}
