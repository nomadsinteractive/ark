#ifndef ARK_GRAPHICS_BASE_RENDER_LAYER_SNAPSHOT_H_
#define ARK_GRAPHICS_BASE_RENDER_LAYER_SNAPSHOT_H_

#include "core/base/api.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/rect.h"
#include "graphics/inf/renderable.h"

#include "renderer/base/buffer.h"

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

public:
    RenderLayerSnapshot(RenderLayerSnapshot&& other) = default;

    sp<RenderCommand> render(const RenderRequest& renderRequest, const V3& position);

    bool needsReload() const;
    const sp<PipelineInput>& pipelineInput() const;

    void addSnapshot(const LayerContext& lc, Renderable::Snapshot snapshot);

    sp<RenderLayer::Stub> _stub;

    size_t _index_count;

    std::vector<UBOSnapshot> _ubos;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;
    std::deque<Renderable::Snapshot> _items;

    Buffer::Snapshot _index_buffer;

    Rect _scissor;

    SnapshotFlag _flag;

    DISALLOW_COPY_AND_ASSIGN(RenderLayerSnapshot);

private:
    RenderLayerSnapshot(RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub);

    friend class RenderLayer;
};

}

#endif
