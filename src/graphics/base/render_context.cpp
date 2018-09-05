#include "graphics/base/render_context.h"

namespace ark {

RenderContext::RenderContext()
    : _last_rendered_count(0), _render_requested(false)
{
}

void RenderContext::renderRequest(const V2& position)
{
    _render_requested = true;
    _position = position;
}

void RenderContext::addRenderObject(const sp<RenderObject>& renderObject)
{
    addRenderObject(renderObject, renderObject.as<Lifecycle>());
}

void RenderContext::addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& expired)
{
    if(expired)
        _items.push_back(renderObject, expired);
    else
        _items.push_back(renderObject, renderObject.as<Lifecycle>());

    _last_rendered_count = -1;
}

void RenderContext::removeRenderObject(const sp<RenderObject>& renderObject)
{
    _items.remove(renderObject);
    _last_rendered_count = -1;
}

void RenderContext::clear()
{
    _items.clear();
}

bool RenderContext::takeSnapshot(LayerContext::Snapshot& output, MemoryPool& memoryPool)
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

RenderContext::RenderObjectVaildator::RenderObjectVaildator(const sp<RenderObject>& /*obj*/, const sp<Boolean>& expired)
    : _expired(expired)
{
}

bool RenderContext::RenderObjectVaildator::operator ()(const sp<RenderObject>& obj) const
{
    return (_expired && _expired->val()) || obj->isExpired();
}

}
