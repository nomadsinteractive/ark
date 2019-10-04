#include "graphics/base/tilemap_layer.h"

#include <algorithm>

#include "core/ark.h"

#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/size.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tileset.h"

namespace ark {

TilemapLayer::TilemapLayer(const Tilemap& tilemap, uint32_t rowCount, uint32_t colCount, const sp<Vec>& position, Tilemap::LayerFlag flag)
    : _col_count(colCount), _row_count(rowCount), _layer_context(tilemap._layer_context), _size(tilemap._size), _tileset(tilemap._tileset), _position(position),
      _scroller(tilemap._scroller), _flag(flag)
{
    _tiles = new sp<RenderObject>[_col_count * _row_count];
}

TilemapLayer::~TilemapLayer()
{
    delete[] _tiles;
}

void TilemapLayer::render(RenderRequest& /*renderRequest*/, const V3& position)
{
    float width = _size->width();
    float height = _size->height();
    float tileWidth = static_cast<float>(_tileset->tileWidth()), tileHeight = static_cast<float>(_tileset->tileHeight());

    const V pos = _position->val();
    const V scroll = _scroller->val();
    float vsx = scroll.x() - pos.x(), vsy = scroll.y() - pos.y();
    float sx, ex, sy, ey, ox, oy;

    if(_flag & Tilemap::LAYER_FLAG_SCROLLABLE)
    {
        viewportIntersect(vsx, vsx + width, _col_count * tileWidth, sx, ex);
        viewportIntersect(vsy, vsy + height, _row_count * tileHeight, sy, ey);
        ox = sx - Math::modFloor(sx, tileWidth) - tileWidth / 2.0f;
        oy = sy - Math::modFloor(sy, tileWidth) - tileHeight / 2.0f;
    }
    else
    {
        sx = sy = 0;
        ex = width;
        ey = height;
        ox = -pos.x() - tileWidth / 2.0f;
        oy = -pos.y() - tileHeight / 2.0f;
    }

    int32_t rowStart = std::max(static_cast<int32_t>(sy / tileHeight) - 1, 0);
    int32_t colStart = std::max(static_cast<int32_t>(sx / tileWidth) - 1, 0);
    int32_t rowEnd = std::min<int32_t>(static_cast<int32_t>(_row_count), static_cast<int32_t>(ey / tileHeight) + 1);
    int32_t colEnd = std::min<int32_t>(static_cast<int32_t>(_col_count), static_cast<int32_t>(ex / tileWidth) + 1);

    for(int32_t i = rowStart; i < rowEnd; ++i)
    {
        float dy = (i - rowStart) * tileHeight - oy;
        for(int32_t j = colStart; j < colEnd; ++j)
        {
            const sp<RenderObject>& renderObject = _tiles[i * _col_count + j];
            if(renderObject)
                _layer_context->drawRenderObject(V3((j - colStart) * tileWidth - ox, dy, 0) + position, renderObject);
        }
    }
}

const sp<Vec>& TilemapLayer::position() const
{
    return _position;
}

void TilemapLayer::setPosition(const sp<Vec>& position)
{
    _position = position;
}

const sp<Tileset>& TilemapLayer::tileset() const
{
    return _tileset;
}

uint32_t TilemapLayer::colCount() const
{
    return _col_count;
}

uint32_t TilemapLayer::rowCount() const
{
    return _row_count;
}

const sp<RenderObject>& TilemapLayer::getTile(uint32_t rowId, uint32_t colId) const
{
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    return _tiles[rowId * _col_count + colId];
}

int32_t TilemapLayer::getTileType(uint32_t rowId, uint32_t colId) const
{
    const sp<RenderObject>& renderObject = getTile(rowId, colId);
    return renderObject ? renderObject->type()->val() : -1;
}

const sp<RenderObject>& TilemapLayer::getTileByPosition(float x, float y) const
{
    return getTile(static_cast<uint32_t>(y / _tileset->tileHeight()), static_cast<uint32_t>(x / _tileset->tileWidth()));
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, const sp<RenderObject>& renderObject)
{
    DCHECK(row < _row_count && col < _col_count, "Invaild tile position:(%d, %d), tilemap size(%d, %d)", row, col, _row_count, _col_count);
    _tiles[row * _col_count + col] = renderObject;
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, int32_t tileId)
{
    const sp<RenderObject>& tile = _tileset->getTile(tileId);
    DCHECK(tile, "TileId %d does not exist", tileId);
    setTile(row, col, tile);
}

Tilemap::LayerFlag TilemapLayer::flag() const
{
    return _flag;
}

void TilemapLayer::setFlag(Tilemap::LayerFlag flag)
{
    _flag = flag;
}

void TilemapLayer::clear()
{
    delete[] _tiles;
    _tiles = new sp<RenderObject>[_col_count * _row_count];
}

void TilemapLayer::viewportIntersect(float vs, float ve, float width, float& start, float& end)
{
    start = std::max(0.0f, vs);
    end = std::min(ve, width);
}

}
