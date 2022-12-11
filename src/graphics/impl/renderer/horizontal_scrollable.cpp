#include "graphics/impl/renderer/horizontal_scrollable.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/bean_utils.h"

#include "graphics/base/v3.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

namespace ark {

HorizontalScrollable::HorizontalScrollable(const sp<RendererMaker>& tileMaker, const sp<Numeric>& scroller, const sp<Size>& size, int32_t tileWidth, uint32_t itemCount)
    : _tile_maker(tileMaker), _tiles(itemCount), _scroller(scroller), _size(size), _width(static_cast<int32_t >(_size->widthAsFloat())), _tile_width(tileWidth),
      _scroll_position(std::numeric_limits<int32_t>::min()), _grid_position(0)
{
    scrollTo(static_cast<int32_t>(_scroller->val()));
}

void HorizontalScrollable::render(RenderRequest& renderRequest, const V3& position)
{
    if(_scroller->update(renderRequest.timestamp()))
        scrollTo(static_cast<int32_t>(_scroller->val()));
    const int32_t gs = _scroll_position - _grid_position;
    const int32_t end = upper(_scroll_position + _width);
    for(int32_t i = _grid_position; i < end; i += _tile_width)
    {
        RendererTile& tile = _tiles[(i - _grid_position) / _tile_width];
        ensureTile(tile, i);
        tile.render(renderRequest, position + V3(static_cast<float>(tile.offset() - gs), 0, 0));
    }
}

const sp<Size>& HorizontalScrollable::size()
{
    return _size;
}

bool HorizontalScrollable::onEvent(const Event& event, float x, float y, bool ptin)
{
    int32_t scrollPosition = static_cast<int32_t>(_scroller->val());
    if(scrollPosition != _scroll_position)
        scrollTo(scrollPosition);
    const int32_t gs = _scroll_position - _grid_position;
    const int32_t end = upper(_scroll_position + _width);
    for(int32_t i = _grid_position; i < end; i += _tile_width)
    {
        RendererTile& tile = _tiles[(i - _grid_position) / _tile_width];
        ensureTile(tile, i);
        if(tile.onEvent(event, x + tile.offset() - gs, y, ptin))
            return true;
    }
    return false;
}

void HorizontalScrollable::scrollTo(int32_t scrollPosition)
{
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

void HorizontalScrollable::ensureTile(RendererTile& tile, int32_t position)
{
    tile.setOffset(position - _grid_position);
    if(tile.position() != position)
    {
        tile.setPosition(position);
//        tile.setRenderer(_tile_maker->make(position, 0));
    }
}

HorizontalScrollable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _renderer_maker(factory.ensureBuilder<RendererMaker>(manifest, "renderer-maker")), _scroller(factory.ensureBuilder<Numeric>(manifest, "scroller")),
      _size(factory.getConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _tile_width(factory.ensureBuilder<Numeric>(manifest, "tile-width")),
      _cols(Documents::getAttribute<uint32_t>(manifest, "cols", 0))
{
}

sp<Renderer> HorizontalScrollable::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size->build(args);
    int32_t tileWidth = static_cast<int32_t>(BeanUtils::toFloat(_tile_width, args));
    DCHECK(tileWidth > 0, "Illegal tile-width: %d", tileWidth);
    return sp<HorizontalScrollable>::make(_renderer_maker->build(args), _scroller->build(args), size, tileWidth, _cols ? _cols : (static_cast<int32_t >(size->widthAsFloat()) - 1) / tileWidth + 2);
}

}
