#include "graphics/util/tiles.h"

namespace ark {

RendererTile::RendererTile()
    : _offset(0), _position(-1)
{
}

RendererTile::RendererTile(const sp<Renderer>& renderer, int32_t offset)
    : _renderer(renderer), _layout_event_listener(renderer.as<LayoutEventListener>()), _offset(offset), _position(-1)
{
}

int32_t RendererTile::offset() const
{
    return _offset;
}

void RendererTile::setOffset(int32_t offset)
{
    _offset = offset;
}

void RendererTile::roll(int32_t offset)
{
    _offset += offset;
}

int32_t RendererTile::position() const
{
    return _position;
}

void RendererTile::setPosition(int32_t position)
{
    _position = position;
}

void RendererTile::setRenderer(sp<Renderer> renderer)
{
    DASSERT(renderer);
    _layout_event_listener = renderer.as<LayoutEventListener>();
    _renderer = std::move(renderer);
}

const sp<Renderer>& RendererTile::renderer() const
{
    return _renderer;
}

const sp<LayoutEventListener>& RendererTile::layoutEventListener() const
{
    return _layout_event_listener;
}

ark::RendererTile::operator bool() const
{
    return static_cast<bool>(_renderer);
}

RollingList::RollingList(uint32_t itemCount)
    : _items(new RendererTile[itemCount]), _item_count(itemCount), _head(0)
{
}

RollingList::~RollingList()
{
    delete[] _items;
}

void RollingList::roll(int32_t offset)
{
    _head = (_head + (offset >= 0 ? offset : offset + _item_count)) % _item_count;
}

RendererTile& RollingList::operator[](uint32_t index)
{
    DCHECK(index < _item_count, "Index out of bounds");
    return _items[(_head + index) % _item_count];
}

const RendererTile& RollingList::operator[](uint32_t index) const
{
    DCHECK(index < _item_count, "Index out of bounds");
    return _items[(_head + index) % _item_count];
}

}
