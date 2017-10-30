#include "graphics/impl/frame/vertical_scrollable.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

VerticalScrollable::VerticalScrollable(const sp<TileMaker>& tileMaker, const sp<Numeric>& scroller, int32_t height, int32_t tileHeight, uint32_t itemCount)
    : _tile_maker(tileMaker),  _tiles(itemCount), _scroller(scroller),
      _height(height), _tile_height(tileHeight),
      _scroll_position(static_cast<int32_t>(scroller->val())), _grid_position(lower(_scroll_position)) {
    update();
}

void VerticalScrollable::render(RenderCommandPipeline& pipeline, float x, float y)
{
    int32_t scrollPosition = static_cast<int32_t>(_scroller->val());
    if(scrollPosition != _scroll_position)
        scrollTo(scrollPosition);
    DCHECK(_scroll_position >= _grid_position, "Illegal scroll position, call update() first");
    const int32_t gs = _scroll_position - _grid_position;
    const int32_t end = upper(_scroll_position + _height);
    for(int32_t i = _grid_position; i < end; i += _tile_height)
    {
        Tile<sp<Renderer>>& tile = _tiles[(i - _grid_position) / _tile_height];
        ensureTile(tile, i);
        tile.renderer()->render(pipeline, x, y + tile.offset() - gs);
    }
}

void VerticalScrollable::scrollTo(int32_t scrollPosition) {
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

void VerticalScrollable::ensureTile(Tile<sp<Renderer>>& tile, int32_t position)
{
    tile.setOffset(position - _grid_position);
    if(tile.position() != position)
    {
        tile.setPosition(position);
        tile.renderer() = _tile_maker->makeTile(0, position);
    }
}

VerticalScrollable::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _tile_maker(parent.ensureBuilder<TileMaker>(doc, "tile-maker")), _scroller(parent.ensureBuilder<Numeric>(doc, "scroller")),
      _height(Documents::ensureAttribute<int32_t>(doc, "height")),
      _tile_height(Documents::ensureAttribute<int32_t>(doc, "tile-height")),
      _rows(Documents::getAttribute<uint32_t>(doc, "rows", 0))
{
    DCHECK(_height > 0, "Illegal height: %d", _height);
    DCHECK(_tile_height > 0, "Illegal tile-height: %d", _tile_height);
}

sp<Renderer> VerticalScrollable::BUILDER::build(const sp<Scope>& args)
{
    return sp<VerticalScrollable>::make(_tile_maker->build(args), _scroller->build(args), _height, _tile_height, _rows ? _rows : (_height - 1) / _tile_height  + 2);
}

}
