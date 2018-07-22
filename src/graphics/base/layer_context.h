#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include <vector>

#include "core/base/api.h"
#include "core/collection/expirable_item_list.h"

#include "graphics/base/render_object.h"

namespace ark {

class LayerContext {
public:
    struct Item {
        Item(float x, float y, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        float x, y;
        sp<RenderObject> _render_object;
    };

    struct Snapshot {
        Snapshot(const LayerContext& layerContext, MemoryPool& memoryPool);
        Snapshot(Snapshot&& other) = default;

        std::vector<RenderObject::Snapshot> _items;
        bool _dirty;

        DISALLOW_COPY_AND_ASSIGN(Snapshot);
    };

public:
    LayerContext(const sp<MemoryPool>& memoryPool);

    void draw(float x, float y, const sp<RenderObject>& renderObject);
    void clear();

    sp<RenderContext> makeRenderContext();

    Snapshot snapshot() const;

private:
    std::vector<Item> _items;
    WeakRefList<RenderContext> _render_contexts;

    sp<MemoryPool> _memory_pool;
    size_t _last_rendered_count;

    friend struct Snapshot;
};

}

#endif
