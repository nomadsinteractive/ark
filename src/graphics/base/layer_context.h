#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/expirable_item_list.h"

#include "graphics/base/render_object.h"

namespace ark {
/*
class LayerContext {
public:
    struct Item {
        Item(float x, float y, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        float x, y;
        sp<RenderObject> _render_object;
    };

public:
    LayerContext(const sp<Camera>& camera, const sp<MemoryPool>& memoryPool);

    void draw(float x, float y, const sp<RenderObject>& renderObject);
    void clear();

    sp<RenderContext> makeRenderContext();

private:
    std::vector<Item> _items;
    WeakRefList<RenderContext> _render_contexts;

    sp<Camera> _camera;
    sp<MemoryPool> _memory_pool;
    size_t _last_rendered_count;

    friend struct Snapshot;
    friend class Layer;
};
*/
}

#endif
