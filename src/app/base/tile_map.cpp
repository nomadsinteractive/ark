#include "app/base/tile_map.h"

#include <algorithm>

#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/size.h"
#include "graphics/base/v2.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/layer.h"

namespace ark {

TileMap::TileMap(const sp<Layer>& layer, uint32_t width, uint32_t height, uint32_t tileWidth, uint32_t tileHeight)
    : _layer(layer), _size(sp<Size>::make(static_cast<float>(width), static_cast<float>(height))),
      _tile_size(sp<Size>::make(static_cast<float>(tileWidth), static_cast<float>(tileHeight))),
      _tile_width(tileWidth), _tile_height(tileHeight), _col_count(width / tileWidth), _row_count(height / tileHeight)
{
    DASSERT(_layer);
    _tiles = new sp<RenderObject>[_col_count * _row_count];
}

TileMap::~TileMap()
{
    delete[] _tiles;
}

void TileMap::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    const V position = _position->val();
    const V scroll = _scroller->val();
    float sx = scroll.x() - position.x(), sy = scroll.y() - position.y();
    float fx, cx, fy, cy;
    Math::modBetween(sx, sx + _size->width(), static_cast<float>(_tile_width), fx, cx);
    Math::modBetween(sy, sy + _size->height(), static_cast<float>(_tile_height), fy, cy);
    int32_t rowIdStart = static_cast<int32_t>(fy / _tile_height);
    int32_t colIdStart = static_cast<int32_t>(fx / _tile_width);
    int32_t rowIdEnd = std::min<int32_t>(_row_count, static_cast<int32_t>(cy / _tile_height));
    int32_t colIdEnd = std::min<int32_t>(_col_count, static_cast<int32_t>(cx / _tile_width));

    float ox = (sx == fx ? 0 : sx - fx) - _tile_width / 2.0f, oy = (sy == fy ? 0 : sy - fy) - _tile_height / 2.0f;
    for(int32_t i = rowIdStart; i < rowIdEnd; i++)
    {
        if(i >= 0)
        {
            float dy = (i - rowIdStart) * _tile_height - oy;
            for(int32_t j = colIdStart; j < colIdEnd; j++)
                if(j >= 0)
                {
                    const sp<RenderObject>& renderObject = _tiles[i * _col_count + j];
                    if(renderObject)
                        _layer->draw(x + (j - colIdStart) * _tile_width - ox, y + dy, renderObject);
                }
        }
    }
}

const SafePtr<Size>& TileMap::size()
{
    return _size;
}

const sp<RenderObject>& TileMap::getTile(uint32_t rowId, uint32_t colId) const
{
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    return _tiles[rowId * _col_count + colId];
}

int32_t TileMap::getTileType(uint32_t rowId, uint32_t colId) const
{
    const sp<RenderObject>& renderObject = getTile(rowId, colId);
    return renderObject ? renderObject->type()->val() : -1;
}

const sp<RenderObject>& TileMap::getTileByPosition(float x, float y) const
{
    return getTile(static_cast<uint32_t>(y / _tile_height), static_cast<uint32_t>(x / _tile_width));
}

void TileMap::setTile(uint32_t rowId, uint32_t colId, const sp<RenderObject>& renderObject)
{
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    _tiles[rowId * _col_count + colId] = renderObject;
}

uint32_t TileMap::tileWidth() const
{
    return _tile_width;
}

uint32_t TileMap::tileHeight() const
{
    return _tile_height;
}

uint32_t TileMap::colCount() const
{
    return _col_count;
}

uint32_t TileMap::rowCount() const
{
    return _row_count;
}

const sp<Vec>& TileMap::position() const
{
    return _position;
}

void TileMap::setPosition(const sp<Vec>& position)
{
    _position = position;
}

const sp<Vec>& TileMap::scroller() const
{
    return _scroller;
}

void TileMap::setScroller(const sp<Vec>& scroller)
{
    _scroller = scroller;
}

}
