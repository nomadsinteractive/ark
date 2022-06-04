#include "graphics/util/tiles.h"

#include "graphics/inf/renderer.h"

namespace ark {

RendererTile::RendererTile()
    : _offset(0), _position(-1)
{
}

RendererTile::RendererTile(std::vector<sp<Renderer> > renderers, int32_t offset)
    : _renderers(std::move(renderers)), _layout_event_listeners(makeLayoutEventListeners()), _offset(offset), _position(-1)
{
}

bool RendererTile::onEvent(const Event& event, float x, float y, bool ptin)
{
    for(const sp<LayoutEventListener>& i : _layout_event_listeners)
        if(i->onEvent(event, x, y, ptin))
            return true;
    return false;
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

void RendererTile::setRenderer(std::vector<sp<Renderer>> renderers)
{
    _renderers = std::move(renderers);
    _layout_event_listeners = makeLayoutEventListeners();
}

void RendererTile::render(RenderRequest& renderRequest, const V3& position)
{
    for(const sp<Renderer>& i : _renderers)
        i->render(renderRequest, position);
}

std::vector<sp<LayoutEventListener>> RendererTile::makeLayoutEventListeners() const
{
    std::vector<sp<LayoutEventListener>> listeners;
    for(const sp<Renderer>& i : _renderers)
    {
        sp<LayoutEventListener> layoutEventListener = i.as<LayoutEventListener>();
        if(layoutEventListener)
            listeners.push_back(std::move(layoutEventListener));
    }
    return listeners;
}

RendererTile::operator bool() const
{
    return _renderers.size() > 0;
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
    _head = (_head + (offset >= 0 ? static_cast<uint32_t>(offset) : offset + _item_count)) % _item_count;
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
