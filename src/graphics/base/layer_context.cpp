#include "graphics/base/layer_context.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : x(x), y(y), _render_object(renderObject)
{
}

void LayerContext::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _items.push_back(Item(x, y, renderObject));
}

void LayerContext::clear()
{
    _items.clear();
}

LayerContext::Snapshot LayerContext::snapshot() const
{
    return Snapshot(*this);
}

LayerContext::Snapshot::Snapshot(const LayerContext& layerContext)
{
    for(const LayerContext::Item& i : layerContext._items)
    {
        RenderObject::Snapshot snapshot = i._render_object->snapshot();
        snapshot._position = V(snapshot._position.x() + i.x, snapshot._position.y() + i.y, snapshot._position.z());
        _items.push_back(snapshot);
    }
}

}
