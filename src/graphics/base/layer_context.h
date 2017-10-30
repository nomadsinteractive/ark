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
        Item(const Item& other);
        Item(Item&& other);

        float x, y;
        sp<RenderObject> renderObject;
    };

public:

    const std::list<Item>& items() const;

    void draw(const std::list<Item>& items);
    void draw(float x, float y, const sp<RenderObject>& renderObject);
    void clear();

private:
    std::list<Item> _items;
};

}

#endif
