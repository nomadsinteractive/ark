#include "graphics/base/layer_context.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : x(x), y(y), _render_object(renderObject)
{
}

LayerContext::LayerContext(const sp<MemoryPool>& memoryPool)
    : _memory_pool(memoryPool)
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
    return Snapshot(*this, _memory_pool);
}

LayerContext::Snapshot::Snapshot(const LayerContext& layerContext, MemoryPool& memoryPool)
{
    for(const LayerContext::Item& i : layerContext._items)
    {
        RenderObject::Snapshot snapshot = i._render_object->snapshot(memoryPool);
        snapshot._position = V(snapshot._position.x() + i.x, snapshot._position.y() + i.y, snapshot._position.z());
        _items.push_back(snapshot);
    }
}

}
