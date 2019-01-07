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

void Scrollable::RollingAdapter::roll(const Scrollable::RollingAdapter& rollingView, TileMaker& tileMaker, int32_t tileWidth, int32_t tileHeight)
{
    int32_t viewRight = rollingView.scrollX() + _params._col_count * tileWidth;
    int32_t viewBottom = rollingView.scrollY() + _params._row_count * tileHeight;
    sp<Renderer>* tiles = _tiles->buf();
    for(int32_t i = 0; i < _params._row_count; i++)
        for(int32_t j = 0; j < _params._col_count; j++)
        {
            int32_t x = _scroll_x + j * tileWidth;
            int32_t y = _scroll_y + i * tileHeight;
            if(x >= rollingView.scrollX() && x < viewRight && y >= rollingView.scrollY() && y < viewBottom)
                tiles[i * _params._col_count + j] = rollingView.getTile((y - rollingView.scrollY()) / tileHeight, (x - rollingView.scrollX()) / tileWidth);
            else
                tiles[i * _params._col_count + j] = tileMaker.makeTile(x, y);
        }
}

const sp<Renderer>& Scrollable::RollingAdapter::getTile(int32_t rowIndex, int32_t colIndex) const
{
    DCHECK(rowIndex >= 0 && colIndex >= 0 && rowIndex < _params._row_count && colIndex < _params._row_count, "Grid(row:%d, col: %d) out of bounds", rowIndex, colIndex);
    return _tiles->buf()[rowIndex * _params._col_count + colIndex];
}

void Scrollable::RollingAdapter::putTile(int32_t rowIndex, int32_t colIndex, const sp<Renderer>& tile)
{
    DCHECK(rowIndex >= 0 && colIndex >= 0 && rowIndex < _params._row_count && colIndex < _params._row_count, "Grid(row:%d, col: %d) out of bounds", rowIndex, colIndex);
    _tiles->buf()[rowIndex * _params._col_count + colIndex] = tile;
}

Scrollable::Scrollable(const sp<Vec>& scroller, const sp<TileMaker>& tileMaker, const sp<Size>& size, const Scrollable::Params& params)
    : _params(params), _rolling_view(RollingAdapter(_params), RollingAdapter(_params)), _scroller(scroller), _tile_maker(tileMaker), _size(size),
      _scroll_x(0), _scroll_y(0)
{
}

void Scrollable::render(RenderRequest& renderRequest, float x, float y)
{
    update();

    const RollingAdapter& rollingView = _rolling_view.front();
    const int32_t cx = _scroll_x - rollingView.scrollX();
    const int32_t cy = _scroll_y - rollingView.scrollY();
    int32_t sx, ex;
    int32_t sy, ey;
    Math::modBetween<int32_t>(cx, cx + width(), _params._tile_width, sx, ex);
    Math::modBetween<int32_t>(cy, cy + height(), _params._tile_height, sy, ey);
    for(int32_t i = sy / _params._tile_height; i < ey / _params._tile_height && i < _params._row_count; i++)
        for(int32_t j = sx / _params._tile_width; j < ex / _params._tile_width && j < _params._col_count; j++)
        {
            const sp<Renderer>& tile = rollingView.getTile(i, j);
            DCHECK(tile, "Grid(row:%d, col: %d) is null", i, j);
            tile->render(renderRequest, x + j * _params._tile_width - cx, y + i * _params._tile_height - cy);
        }
}

const SafePtr<Size>& Scrollable::size()
{
    return _size;
}

void Scrollable::initialize()
{
    RollingAdapter& rollingView = _rolling_view.back();

    rollingView.setScrollXY(_scroll_x - _params._col_index * _params._tile_width, _scroll_y - _params._row_index * _params._tile_height);

    for(int32_t i = 0; i < _params._row_count; i++)
        for(int32_t j = 0; j < _params._col_count; j++)
            rollingView.putTile(i, j, _tile_maker->makeTile((j - _params._col_index) * _params._tile_width, (i - _params._row_index) * _params._tile_height));

    _rolling_view.swap();
}

void Scrollable::updateTask()
{
    const RollingAdapter& front = _rolling_view.front();
    const int32_t colIndex = (_scroll_x - front.scrollX()) / _params._tile_width;
    const int32_t rowIndex = (_scroll_y - front.scrollY()) / _params._tile_height;
    if(colIndex != _params._col_index || rowIndex != _params._row_index)
    {
        LOGD("col = %d row = %d", colIndex, rowIndex);
        RollingAdapter& back = _rolling_view.back();
        back.setScrollXY(Math::modFloor<int32_t>(_scroll_x - _params._col_index * _params._tile_width, _params._tile_width), Math::modFloor(_scroll_y - _params._row_index * _params._tile_height, _params._tile_height));
        back.roll(front, _tile_maker, _params._tile_width, _params._tile_height);
        _rolling_view.swap();
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

Scrollable::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _scroller(parent.ensureBuilder<Vec>(doc, "scroller")), _tile_maker(parent.ensureBuilder<TileMaker>(doc, "tile-maker")), _size(parent.ensureBuilder<Size>(doc, Constants::Attributes::SIZE)),
      _rows(Documents::ensureAttribute<int32_t>(doc, "rows")), _cols(Documents::ensureAttribute<int32_t>(doc, "cols")),
      _row_index(_rows / 2), _col_index(_cols / 2),
      _tile_width(Documents::ensureAttribute<int32_t>(doc, "tile-width")), _tile_height(Documents::ensureAttribute<int32_t>(doc, "tile-height"))
{
}

sp<Renderer> Scrollable::BUILDER::build(const sp<Scope>& args)
{
    const Scrollable::Params params(_rows, _cols, _row_index, _col_index, _tile_width, _tile_height);
    const sp<Scrollable> scrollable = sp<Scrollable>::make(_scroller->build(args), _tile_maker->build(args), _size->build(args), params);
    scrollable->initialize();
    return scrollable;
}

}
