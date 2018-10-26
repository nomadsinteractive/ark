#include "graphics/base/layer_context.h"

namespace ark {

LayerContext::LayerContext()
    : _last_rendered_count(0), _render_requested(false)
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

void LayerContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& lifecycle)
{
    if(lifecycle)
        _items.push_back(renderObject, lifecycle);
    else
        _items.push_back(renderObject, renderObject.as<Lifecycle>());

    _last_rendered_count = -1;
}

void LayerContext::removeRenderObject(const sp<RenderObject>& renderObject)
{
    _items.remove(renderObject);
    _last_rendered_count = -1;
}

void LayerContext::clear()
{
    _items.clear();
}

bool LayerContext::takeSnapshot(Layer::Snapshot& output, MemoryPool& memoryPool)
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
    bool dirty = renderedCount != _last_rendered_count;
    _last_rendered_count = renderedCount;
    _render_requested = false;
    return dirty;
}

LayerContext::RenderObjectVaildator::RenderObjectVaildator(const sp<RenderObject>& /*obj*/, const sp<Boolean>& disposed)
    : _disposed(disposed)
{
}

bool LayerContext::RenderObjectVaildator::operator ()(const sp<RenderObject>& obj) const
{
    return (_disposed && _disposed->val()) || obj->isDisposed();
}

}
