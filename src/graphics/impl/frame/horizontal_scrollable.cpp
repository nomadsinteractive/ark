#include "graphics/impl/frame/horizontal_scrollable.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/bean_utils.h"

namespace ark {

HorizontalScrollable::HorizontalScrollable(const sp<TileMaker>& tileMaker, const sp<Numeric>& scroller, int32_t width, int32_t tileWidth, uint32_t itemCount)
    : _tile_maker(tileMaker),  _tiles(itemCount), _scroller(scroller), _width(width), _tile_width(tileWidth),
      _scroll_position(std::numeric_limits<int32_t>::min()), _grid_position(0) {
}

void HorizontalScrollable::render(RenderRequest& renderRequest, float x, float y)
{
    int32_t scrollPosition = static_cast<int32_t>(_scroller->val());
    if(scrollPosition != _scroll_position)
        scrollTo(scrollPosition);
    const int32_t gs = _scroll_position - _grid_position;
    const int32_t end = upper(_scroll_position + _width);
    for(int32_t i = _grid_position; i < end; i += _tile_width)
    {
        Tile<sp<Renderer>>& tile = _tiles[(i - _grid_position) / _tile_width];
        ensureTile(tile, i);
        tile.renderer()->render(renderRequest, x + tile.offset() - gs, y);
    }
}

void HorizontalScrollable::scrollTo(int32_t scrollPosition) {
    int32_t gridPosition = lower(scrollPosition);
    _scroll_position = scrollPosition;
    if(gridPosition != _grid_position)
    {
        _tiles.roll((gridPosition - _grid_position) / _tile_width);
        _grid_position = gridPosition;
    }
}

int32_t HorizontalScrollable::lower(int32_t pos) const
{
    int32_t mod = pos % _tile_width;
    return pos - (mod >= 0 ? mod : _tile_width + mod);
}

int32_t HorizontalScrollable::upper(int32_t pos) const
{
    int32_t mod = pos % _tile_width;
    return mod == 0 ? pos : pos - (mod >= 0 ? mod : _tile_width + mod) + _tile_width;
}

void HorizontalScrollable::ensureTile(Tile<sp<Renderer>>& tile, int32_t position)
{
    tile.setOffset(position - _grid_position);
    if(tile.position() != position)
    {
        const sp<Renderer> renderer = _tile_maker->makeTile(position, 0);
        DASSERT(renderer);
        tile.setPosition(position);
        tile.renderer() = renderer;
    }
}

HorizontalScrollable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _tile_maker(factory.ensureBuilder<TileMaker>(manifest, "tile-maker")), _scroller(factory.ensureBuilder<Numeric>(manifest, "scroller")),
      _width(factory.ensureBuilder<Numeric>(manifest, "width")),
      _tile_width(factory.ensureBuilder<Numeric>(manifest, "tile-width")),
      _cols(Documents::getAttribute<uint32_t>(manifest, "cols", 0))
{
}

sp<Renderer> HorizontalScrollable::BUILDER::build(const sp<Scope>& args)
{
    int32_t width = static_cast<int32_t>(BeanUtils::toFloat(_width, args));
    int32_t tileWidth = static_cast<int32_t>(BeanUtils::toFloat(_tile_width, args));
    DCHECK(width > 0, "Illegal width: %d", width);
    DCHECK(tileWidth > 0, "Illegal tile-width: %d", tileWidth);
    return sp<HorizontalScrollable>::make(_tile_maker->build(args), _scroller->build(args), width, tileWidth, _cols ? _cols : (width - 1) / tileWidth  + 2);
}

}
