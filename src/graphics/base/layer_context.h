#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include <list>

#include "core/base/api.h"

#include "graphics/base/render_object.h"

namespace ark {

class LayerContext {
public:
    struct Item {
        Item(float x, float y, const sp<RenderObject>& renderObject);
        Item(const Item& other) = default;

        float x, y;
        sp<RenderObject> _render_object;
    };

    struct Snapshot {
        Snapshot(const LayerContext& layerContext);
        Snapshot(Snapshot&& other) = default;

        std::list<RenderObject::Snapshot> _items;

        DISALLOW_COPY_AND_ASSIGN(Snapshot);
    };

public:
    void draw(float x, float y, const sp<RenderObject>& renderObject);
    void clear();

    Snapshot snapshot() const;

private:
    std::list<Item> _items;

    friend struct Snapshot;
};

}

#endif
