#include "graphics/impl/frame/scrollable.h"

#include "core/base/bean_factory.h"
#include "core/inf/array.h"
#include "core/inf/message_loop.h"
#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/size.h"
#include "graphics/base/v2.h"

namespace ark {

Scrollable::RollingAdapter::RollingAdapter(const Scrollable::Params& params)
    : _params(params), _tiles(sp<Array<sp<Renderer>>::Allocated>::make(params._col_count * params._row_count)), _scroll_x(0), _scroll_y(0)
{
}

Scrollable::RollingAdapter::RollingAdapter(const Scrollable::RollingAdapter& other)
    : _params(other._params), _tiles(other._tiles), _scroll_x(other._scroll_x), _scroll_y(other._scroll_y)
{
}

void Scrollable::RollingAdapter::setScrollXY(int32_t x, int32_t y)
{
    _scroll_x = x;
    _scroll_y = y;
}

int32_t Scrollable::RollingAdapter::scrollX() const
{
    return _scroll_x;
}

int32_t Scrollable::RollingAdapter::scrollY() const
{
    return _scroll_y;
}

void Scrollable::RollingAdapter::roll(const Scrollable::RollingAdapter& rollingView, RendererMaker& tileMaker, int32_t tileWidth, int32_t tileHeight)
{
    int32_t viewRight = rollingView.scrollX() + _params._col_count * tileWidth;
    int32_t viewBottom = rollingView.scrollY() + _params._row_count * tileHeight;
    sp<Renderer>* tiles = _tiles->buf();

    std::set<sp<Renderer>> overrided;
    std::set<sp<Renderer>> maked;

    for(int32_t i = 0; i < _params._row_count; i++)
        for(int32_t j = 0; j < _params._col_count; j++)
        {
            sp<Renderer>& renderer = tiles[i * _params._col_count + j];
            int32_t x = _scroll_x + j * tileWidth;
            int32_t y = _scroll_y + i * tileHeight;
            overrided.insert(renderer);
            if(x >= rollingView.scrollX() && x < viewRight && y >= rollingView.scrollY() && y < viewBottom)
                renderer = rollingView.getTile(tileMaker, (y - rollingView.scrollY()) / tileHeight, (x - rollingView.scrollX()) / tileWidth);
            else
                renderer = tileMaker.make(x, y);
            maked.insert(renderer);
        }

    for(const sp<Renderer>& i : overrided)
        if(i && maked.find(i) != maked.end())
            tileMaker.recycle(i);
}

const sp<Renderer>& Scrollable::RollingAdapter::getTile(RendererMaker& tileMaker, int32_t rowIndex, int32_t colIndex) const
{
    DCHECK(rowIndex >= 0 && colIndex >= 0 && rowIndex < _params._row_count && colIndex < _params._row_count, "Grid(row:%d, col: %d) out of bounds", rowIndex, colIndex);
    sp<Renderer>& renderer = _tiles->buf()[rowIndex * _params._col_count + colIndex];
    if(!renderer)
    {
        int32_t x = _scroll_x + colIndex * _params._tile_width;
        int32_t y = _scroll_y + rowIndex * _params._tile_height;
        renderer = tileMaker.make(x, y);
    }
    return renderer;
}

void Scrollable::RollingAdapter::putTile(int32_t rowIndex, int32_t colIndex, const sp<Renderer>& tile)
{
    DCHECK(rowIndex >= 0 && colIndex >= 0 && rowIndex < _params._row_count && colIndex < _params._row_count, "Grid(row:%d, col: %d) out of bounds", rowIndex, colIndex);
    _tiles->buf()[rowIndex * _params._col_count + colIndex] = tile;
}

Scrollable::Scrollable(const sp<Vec>& scroller, const sp<RendererMaker>& tileMaker, const sp<Size>& size, const Scrollable::Params& params)
    : _params(params), _rolling_view(_params), _scroller(scroller), _renderer_maker(tileMaker), _size(size),
      _scroll_x(0), _scroll_y(0)
{
}

void Scrollable::render(RenderRequest& renderRequest, const V3& position)
{
    update();

    const int32_t cx = _scroll_x - _rolling_view.scrollX();
    const int32_t cy = _scroll_y - _rolling_view.scrollY();
    int32_t sx, ex;
    int32_t sy, ey;
    Math::modBetween<int32_t>(cx, cx + width(), _params._tile_width, sx, ex);
    Math::modBetween<int32_t>(cy, cy + height(), _params._tile_height, sy, ey);
    for(int32_t i = sy / _params._tile_height; i <= ey / _params._tile_height && i < _params._row_count; i++)
        for(int32_t j = sx / _params._tile_width; j <= ex / _params._tile_width && j < _params._col_count; j++)
        {
            const sp<Renderer>& tile = _rolling_view.getTile(_renderer_maker, i, j);
            DCHECK(tile, "Grid(row:%d, col: %d) is null", i, j);
            tile->render(renderRequest, V3(j * _params._tile_width - cx, i * _params._tile_height - cy, 0) + position);
        }
}

const sp<Size>& Scrollable::size()
{
    return _size;
}

void Scrollable::updateTask()
{
    const int32_t colIndex = (_scroll_x - _rolling_view.scrollX()) / _params._tile_width;
    const int32_t rowIndex = (_scroll_y - _rolling_view.scrollY()) / _params._tile_height;
    if(colIndex != _params._col_index || rowIndex != _params._row_index)
    {
        const RollingAdapter front = _rolling_view;
        LOGD("col = %d row = %d", colIndex, rowIndex);
        _rolling_view.setScrollXY(Math::modFloor<int32_t>(_scroll_x - _params._col_index * _params._tile_width, _params._tile_width), Math::modFloor(_scroll_y - _params._row_index * _params._tile_height, _params._tile_height));
        _rolling_view.roll(front, _renderer_maker, _params._tile_width, _params._tile_height);
    }
}

void Scrollable::update()
{
    const V scroll = _scroller->val();
    int32_t scrollX = static_cast<int32_t>(scroll.x());
    int32_t scrollY = static_cast<int32_t>(scroll.y());
    if(scrollX != _scroll_x || scrollY != _scroll_y)
    {
        _scroll_x = scrollX;
        _scroll_y = scrollY;
        updateTask();
    }
}

int32_t Scrollable::width() const
{
    return static_cast<int32_t>(_size->width());
}

int32_t Scrollable::height() const
{
    return static_cast<int32_t>(_size->height());
}

Scrollable::Params::Params(int32_t rowCount, int32_t colCount, int32_t rowIndex, int32_t colIndex, int32_t tileWidth, int32_t tileHeight)
    : _row_count(rowCount), _col_count(colCount), _row_index(rowIndex), _col_index(colIndex), _tile_width(tileWidth), _tile_height(tileHeight)
{
}

Scrollable::Params::Params(const document& manifest)
    : _row_count(Documents::ensureAttribute<int32_t>(manifest, "rows")), _col_count(Documents::ensureAttribute<int32_t>(manifest, "cols")),
      _row_index(_row_count / 2), _col_index(_col_count / 2), _tile_width(Documents::ensureAttribute<int32_t>(manifest, "renderer-width")),
      _tile_height(Documents::ensureAttribute<int32_t>(manifest, "renderer-height"))
{
}

Scrollable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _scroller(factory.ensureBuilder<Vec>(manifest, "scroller")), _tile_maker(factory.ensureBuilder<RendererMaker>(manifest, "renderer-maker")),
      _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)), _params(manifest)
{
}

sp<Renderer> Scrollable::BUILDER::build(const Scope& args)
{
    return sp<Scrollable>::make(_scroller->build(args), _tile_maker->build(args), _size->build(args), _params);
}

}
