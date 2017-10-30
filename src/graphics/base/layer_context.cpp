#include "graphics/base/layer_context.h"

namespace ark {

LayerContext::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : x(x), y(y), renderObject(renderObject)
{
}

LayerContext::Item::Item(const LayerContext::Item& other)
    : x(other.x), y(other.y), renderObject(other.renderObject)
{
}

LayerContext::Item::Item(LayerContext::Item&& other)
    : x(other.x), y(other.y), renderObject(std::move(other.renderObject))
{
}

const std::list<LayerContext::Item>& LayerContext::items() const
{
    return _items;
}

void LayerContext::draw(const std::list<LayerContext::Item>& items)
{
    for(const LayerContext::Item& i : items)
        _items.push_back(i);
}

void LayerContext::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _items.push_back(Item(x, y, renderObject));
}

void LayerContext::clear()
{
    _items.clear();
}

}
