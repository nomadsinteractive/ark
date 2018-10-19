#include "graphics/base/layer_context.h"

#include "graphics/base/render_context.h"

namespace ark {
/*
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
*/
}
