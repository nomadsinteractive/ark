#pragma once

#include <deque>
#include <list>
#include <vector>

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

    struct BufferObject {
        std::vector<UBOSnapshot> _ubos;
        std::vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;
    };

    enum SnapshotFlag {
        SNAPSHOT_FLAG_RELOAD,
        SNAPSHOT_FLAG_DYNAMIC_UPDATE,
        SNAPSHOT_FLAG_STATIC_MODIFIED,
        SNAPSHOT_FLAG_STATIC_REUSE
    };

    struct Element {
        Element(Renderable& renderable, const LayerContextSnapshot& layerContext, LayerContext::ElementState& state, const Renderable::Snapshot& snapshot);

        const Renderable::Snapshot& ensureSnapshot(const RenderRequest& renderRequest, bool reload);

        Renderable& _renderable;
        const LayerContextSnapshot& _layer_context;
        LayerContext::ElementState& _element_state;
        Renderable::Snapshot _snapshot;
    };

public:
    RenderLayerSnapshot(RenderLayerSnapshot&& other) = default;

    sp<RenderCommand> compose(const RenderRequest& renderRequest) const;

    bool needsReload() const;
    const sp<PipelineInput>& pipelineInput() const;

    void addLayerContext(const RenderRequest& renderRequest, std::vector<sp<LayerContext>>& layerContexts);
    void snapshot(const RenderRequest& renderRequest);

    sp<RenderCommand> toRenderCommand(const RenderRequest& renderRequest, Buffer::Snapshot vertices, Buffer::Snapshot indices, uint32_t drawCount, DrawingParams params) const;

    sp<RenderLayer::Stub> _stub;

    size_t _index_count;
    sp<BufferObject> _buffer_object;

    std::deque<Element> _elements;
    std::deque<LayerContext::ElementState> _elements_deleted;
    Rect _scissor;
    bool _vertices_dirty;

    DISALLOW_COPY_AND_ASSIGN(RenderLayerSnapshot);

private:
    std::list<LayerContextSnapshot> _layer_context_snapshots;

private:
    RenderLayerSnapshot(const RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub);

    bool doAddLayerContext(const RenderRequest& renderRequest, LayerContext& layerContext);
    bool addDiscardedState(LayerContext& lc, void* stateKey);
    void addDiscardedLayerContext(LayerContext& lc);
    void addDiscardedLayerContexts(const std::vector<sp<LayerContext>>& layerContexts);

    friend class RenderLayer;
};

}
