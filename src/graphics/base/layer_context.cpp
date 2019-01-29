#include "graphics/base/layer_context.h"

#include "core/epi/lifecycle.h"

namespace ark {

LayerContext::LayerContext()
    : _render_requested(false)
{
}

void LayerContext::renderRequest(const V2& position)
{
    _render_requested = true;
    _position = position;
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject)
{
    addRenderObject(renderObject, renderObject.as<Lifecycle>());
}

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Lifecycle>& lifecycle)
{
    if(lifecycle)
        _items.push_back(renderObject, lifecycle);
    else
        _items.push_back(renderObject, renderObject.as<Lifecycle>());
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
    size_t renderedCount = 0;
    if(_render_requested)
        for(const sp<RenderObject>& i : _items)
        {
            ++renderedCount;
            RenderObject::Snapshot snapshot = i->snapshot(memoryPool);
            snapshot._position = V(snapshot._position.x() + _position.x(), snapshot._position.y() + _position.y(), snapshot._position.z());
            output._items.push_back(std::move(snapshot));
        }
    _render_requested = false;
}

LayerContext::RenderObjectFilter::RenderObjectFilter(const sp<RenderObject>& /*renderObject*/, const sp<Lifecycle>& disposed)
    : _lifecycle(disposed)
{
}

FilterAction LayerContext::RenderObjectFilter::operator()(const sp<RenderObject>& renderObject) const
{
    return (_lifecycle && _lifecycle->isDisposed()) || renderObject->isDisposed() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
}

}
