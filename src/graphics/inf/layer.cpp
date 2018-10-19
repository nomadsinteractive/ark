#include "graphics/inf/layer.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"

namespace ark {

Layer::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : x(x), y(y), _render_object(renderObject)
{
}

/*
Layer::Stub::Stub(const sp<Camera>& camera, const sp<MemoryPool>& memoryPool)
    : _camera(camera), _memory_pool(memoryPool), _last_rendered_count(-1)
{
}

void Layer::Stub::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _items.push_back(Item(x, y, renderObject));
}

void Layer::Stub::clear()
{
    _last_rendered_count = _items.size();
    _items.clear();
}

sp<RenderContext> Layer::Stub::makeRenderContext()
{
    const sp<RenderContext> collection = sp<RenderContext>::make();
    _render_contexts.push_back(collection);
    return collection;
}
*/
Layer::Snapshot::Snapshot(const Layer& layer, MemoryPool& memoryPool)
    : _camera(layer._camera->snapshop()), _dirty(layer._items.size() != layer._last_rendered_count)
{
    for(const Item& i : layer._items)
    {
        RenderObject::Snapshot snapshot = i._render_object->snapshot(memoryPool);
        snapshot._position = V(snapshot._position.x() + i.x, snapshot._position.y() + i.y, snapshot._position.z());
        _items.push_back(snapshot);
    }

    size_t size = layer._render_contexts.size();
    for(const sp<RenderContext>& i : layer._render_contexts)
        _dirty = i->takeSnapshot(*this, memoryPool) || _dirty;
    _dirty = _dirty || size != layer._render_contexts.size();
}

Layer::Layer(const sp<Camera>& camera, const sp<MemoryPool>& memoryPool)
    : _camera(camera), _memory_pool(memoryPool), _last_rendered_count(-1)
{
}

void Layer::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _items.push_back(Item(x, y, renderObject));
}

void Layer::clear()
{
    _last_rendered_count = _items.size();
    _items.clear();
}

Layer::Snapshot Layer::snapshot() const
{
    return Snapshot(*this, _memory_pool);
}

sp<RenderContext> Layer::makeRenderContext()
{
    const sp<RenderContext> collection = sp<RenderContext>::make();
    _render_contexts.push_back(collection);
    return collection;
}

Layer::Renderer::Renderer(const sp<Layer>& layer)
    : _layer(layer)
{
    DASSERT(_layer);
}

void Layer::Renderer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addBackgroundRequest(_layer, x, y);
}

}
