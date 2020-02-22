#include "graphics/impl/renderer/vertical_scrollable.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"

#include "graphics/base/v3.h"
#include "graphics/impl/vec/vec4_impl.h"

#include "app/base/event.h"
#include "app/view/layout_hierarchy.h"

namespace ark {

VerticalScrollable::VerticalScrollable(const sp<RendererMaker>& tileMaker, const sp<Numeric>& scroller, const sp<Vec4>& scissor, int32_t height, int32_t tileHeight, uint32_t itemCount)
    : _tile_maker(tileMaker),  _tiles(itemCount), _scroller(scroller), _scissor(scissor.as<Vec4Impl>()), _height(height), _tile_height(tileHeight),
      _scroll_position(static_cast<int32_t>(scroller->val())), _grid_position(lower(_scroll_position))
{
    DCHECK(!scissor || _scissor, "Scissor must be an instance of Vec4Impl");
    update();
}

void VerticalScrollable::render(RenderRequest& renderRequest, const V3& position)
{
    int32_t scrollPosition = static_cast<int32_t>(_scroller->val());
    if(scrollPosition != _scroll_position)
        scrollTo(scrollPosition);
    DCHECK(_scroll_position >= _grid_position, "Illegal scroll position, call update() first");
    const int32_t gs = _scroll_position - _grid_position;
    const int32_t end = upper(_scroll_position + _height);
    for(int32_t i = _grid_position; i < end; i += _tile_height)
    {
        RendererTile& tile = _tiles[(i - _grid_position) / _tile_height];
        ensureTile(tile, i);
        tile.render(renderRequest, position + V3(0, tile.offset() - gs, 0));
    }
    if(_scissor)
    {
        _scissor->x()->set(position.x());
        _scissor->y()->set(position.y());
        _scissor->w()->set(position.y() + _height);
    }
}

bool VerticalScrollable::onEvent(const Event& event, float x, float y, bool ptin)
{
    int32_t scrollPosition = static_cast<int32_t>(_scroller->val());
    if(scrollPosition != _scroll_position)
        scrollTo(scrollPosition);
    DCHECK(_scroll_position >= _grid_position, "Illegal scroll position, call update() first");
    const int32_t gs = _scroll_position - _grid_position;
    const int32_t end = upper(_scroll_position + _height);
    for(int32_t i = _grid_position; i < end; i += _tile_height)
    {
        RendererTile& tile = _tiles[(i - _grid_position) / _tile_height];
        ensureTile(tile, i);
        float ty = y + tile.offset() - gs;
        if(tile.layoutEventListener() && tile.layoutEventListener()->onEvent(event, x, ty, ptin && event.y() >= ty && event.y() <= ty + _tile_height))
            return true;
    }
    return false;
}

void VerticalScrollable::scrollTo(int32_t scrollPosition)
{
    int32_t gridPosition = lower(scrollPosition);
    _scroll_position = scrollPosition;
    if(gridPosition != _grid_position)
    {
        _tiles.roll((gridPosition - _grid_position) / _tile_height);
        _grid_position = gridPosition;
    }
}

void VerticalScrollable::update()
{
    const int32_t end = upper(_scroll_position + _height);
    for(int32_t i = _grid_position; i < end; i += _tile_height)
        ensureTile(_tiles[(i - _grid_position) / _tile_height], i);
}

int32_t VerticalScrollable::lower(int32_t pos) const
{
    int32_t mod = pos % _tile_height;
    return pos - (mod >= 0 ? mod : _tile_height + mod);
}

int32_t VerticalScrollable::upper(int32_t pos) const
{
    int32_t mod = pos % _tile_height;
    return mod == 0 ? pos : pos - (mod >= 0 ? mod : _tile_height + mod) + _tile_height;
}

void VerticalScrollable::ensureTile(RendererTile& tile, int32_t position)
{
    tile.setOffset(position - _grid_position);
    if(tile.position() != position)
    {
        tile.setPosition(position);
        tile.setRenderer(_tile_maker->make(0, position));
    }
}

VerticalScrollable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _renderer_maker(factory.ensureBuilder<RendererMaker>(manifest, "renderer-maker")), _scroller(factory.ensureBuilder<Numeric>(manifest, "scroller")),
      _scissor(factory.getBuilder<Vec4>(manifest, "scissor")), _height(Documents::ensureAttribute<int32_t>(manifest, "height")),
      _tile_height(Documents::ensureAttribute<int32_t>(manifest, "tile-height")),
      _rows(Documents::getAttribute<uint32_t>(manifest, "rows", 0))
{
    DCHECK(_height > 0, "Illegal height: %d", _height);
    DCHECK(_tile_height > 0, "Illegal tile-height: %d", _tile_height);
}

sp<Renderer> VerticalScrollable::BUILDER::build(const Scope& args)
{
    return sp<VerticalScrollable>::make(_renderer_maker->build(args), _scroller->build(args), _scissor->build(args), _height, _tile_height,
                                        _rows ? _rows : (_height - 1) / _tile_height  + 2);
}

}
