#include "graphics/base/layer_context.h"

#include "core/epi/disposable.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : _x(x), _y(y), _render_object(renderObject)
{
}

LayerContext::LayerContext()
    : _render_requested(false)
{
}

void LayerContext::renderRequest(const V2& position)
{
    _render_requested = true;
    _position = position;
}

void LayerContext::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _transient_items.emplace_back(x, y, renderObject);
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject)
{
    addRenderObject(renderObject, renderObject.as<Disposable>());
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Disposable>& disposed)
{
    _items.push_back(renderObject, disposed);
}

void LayerContext::removeRenderObject(const sp<RenderObject>& renderObject)
{
    _items.remove(renderObject);
}

void LayerContext::clear()
{
    _items.clear();
}

void LayerContext::takeSnapshot(Layer::Snapshot& output, MemoryPool& memoryPool)
{
    if(_render_requested)
    {
        for(const Item& i : _transient_items)
        {
            RenderObject::Snapshot snapshot = i._render_object->snapshot(memoryPool);
            snapshot._position = V(snapshot._position.x() + i._x, snapshot._position.y() + i._y, snapshot._position.z());
            output._items.push_back(std::move(snapshot));
        }

        for(const sp<RenderObject>& i : _items)
        {
            RenderObject::Snapshot snapshot = i->snapshot(memoryPool);
            snapshot._position = V(snapshot._position.x() + _position.x(), snapshot._position.y() + _position.y(), snapshot._position.z());
            output._items.push_back(std::move(snapshot));
        }
    }
    _transient_items.clear();
    _render_requested = false;
}

LayerContext::RenderObjectFilter::RenderObjectFilter(const sp<RenderObject>& /*renderObject*/, const sp<Disposable>& disposed)
    : _disposed(disposed)
{
}

FilterAction LayerContext::RenderObjectFilter::operator()(const sp<RenderObject>& renderObject) const
{
    return (_disposed && _disposed->isDisposed()) || renderObject->isDisposed() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
}

}
