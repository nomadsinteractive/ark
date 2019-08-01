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

TilemapLayer::TilemapLayer(uint32_t rowCount, uint32_t colCount, const sp<Tileset>& tileset, const sp<Vec>& position, Tilemap::LayerFlag flag)
    : _col_count(colCount), _row_count(rowCount), _tileset(tileset), _position(position), _flag(flag)
{
    _tiles = new sp<RenderObject>[_col_count * _row_count];
}

TilemapLayer::~TilemapLayer()
{
    delete[] _tiles;
}

void TilemapLayer::render(LayerContext& layerContext, const V& scroll, float width, float height)
{
    const V position = _position->val();
    float sx = scroll.x() - position.x(), sy = scroll.y() - position.y();
    float fx, cx, fy, cy, tileWidth = static_cast<float>(_tileset->tileWidth()), tileHeight = static_cast<float>(_tileset->tileHeight());
    Math::modBetween(sx, sx + width, tileWidth, fx, cx);
    Math::modBetween(sy, sy + height, tileHeight, fy, cy);
    int32_t rowStart = static_cast<int32_t>(fy / tileHeight);
    int32_t colStart = static_cast<int32_t>(fx / tileWidth);
    int32_t rowEnd = std::min<int32_t>(static_cast<int32_t>(_row_count), static_cast<int32_t>(cy / tileHeight));
    int32_t colEnd = std::min<int32_t>(static_cast<int32_t>(_col_count), static_cast<int32_t>(cx / tileWidth));
    float ox = sx - fx - tileWidth / 2.0f, oy = sy - fy - tileHeight / 2.0f;

    for(int32_t i = std::max(rowStart, 0); i < rowEnd; ++i)
    {
        float dy = (i - rowStart) * tileHeight - oy;
        for(int32_t j = std::max(colStart, 0); j < colEnd; ++j)
        {
            const sp<RenderObject>& renderObject = _tiles[i * _col_count + j];
            if(renderObject)
                layerContext.draw((j - colStart) * tileWidth - ox, dy, renderObject);
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

}
