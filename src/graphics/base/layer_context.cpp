#include "graphics/base/layer_context.h"

#include "graphics/base/render_context.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : x(x), y(y), _render_object(renderObject)
{
}

LayerContext::LayerContext(const sp<Camera>& camera, const sp<MemoryPool>& memoryPool)
    : _camera(camera), _memory_pool(memoryPool), _last_rendered_count(-1)
{
}

void LayerContext::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _items.push_back(Item(x, y, renderObject));
}

void LayerContext::clear()
{
    _last_rendered_count = _items.size();
    _items.clear();
}

sp<RenderContext> LayerContext::makeRenderContext()
{
    const sp<RenderContext> collection = sp<RenderContext>::make();
    _render_contexts.push_back(collection);
    return collection;
}

LayerContext::Snapshot LayerContext::snapshot() const
{
    return Snapshot(*this, _memory_pool);
}

LayerContext::Snapshot::Snapshot(const LayerContext& layerContext, MemoryPool& memoryPool)
    : _camera(layerContext._camera->snapshop()), _dirty(layerContext._items.size() != layerContext._last_rendered_count)
{
    for(const LayerContext::Item& i : layerContext._items)
    {
        RenderObject::Snapshot snapshot = i._render_object->snapshot(memoryPool);
        snapshot._position = V(snapshot._position.x() + i.x, snapshot._position.y() + i.y, snapshot._position.z());
        _items.push_back(snapshot);
    }

    size_t size = layerContext._render_contexts.size();
    for(const sp<RenderContext>& i : layerContext._render_contexts)
        _dirty = i->takeSnapshot(*this, memoryPool) || _dirty;
    _dirty = _dirty || size != layerContext._render_contexts.size();
}

}
