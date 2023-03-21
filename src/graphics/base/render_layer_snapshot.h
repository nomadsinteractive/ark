#pragma once

#include "core/base/api.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/rect.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context_params.h"

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

    struct SnapshotWithState {
        SnapshotWithState(LayerContext::ElementState& state, Renderable::Snapshot snapshot);

        LayerContext::ElementState& _state;
        Renderable::Snapshot _snapshot;
    };

public:
    RenderLayerSnapshot(RenderLayerSnapshot&& other) = default;

    sp<RenderCommand> compose(const RenderRequest& renderRequest);

    bool needsReload() const;
    const sp<PipelineInput>& pipelineInput() const;

    void loadSnapshot(const LayerContext& lc, Renderable::Snapshot& snapshot, const Varyings::Snapshot& defaultVaryingsSnapshot);
    void addSnapshot(LayerContext& lc, Renderable::Snapshot snapshot, void* stateKey);

    void ensureState(LayerContext& lc, void* stateKey);
    void addDisposedState(LayerContext& lc, void* stateKey);

    void addDisposedLayerContext(LayerContext& lc);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, DrawingContextParams::Parameters params);

    sp<RenderLayer::Stub> _stub;

    size_t _index_count;

    std::vector<UBOSnapshot> _ubos;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;

    std::deque<SnapshotWithState> _items;

    std::deque<LayerContext::ElementState> _item_deleted;

    Buffer::Snapshot _index_buffer;

    Rect _scissor;

    SnapshotFlag _flag;

    DISALLOW_COPY_AND_ASSIGN(RenderLayerSnapshot);

private:

    RenderLayerSnapshot(RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub);

    friend class RenderLayer;
};

}
