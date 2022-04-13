#include "graphics/base/tilemap_layer.h"

#include <algorithm>

#include "core/ark.h"

#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/tile.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tileset.h"

namespace ark {

TilemapLayer::TilemapLayer(const Tilemap& tilemap, String name, uint32_t rowCount, uint32_t colCount, sp<Vec3> position, sp<Vec3> scroller, Tilemap::LayerFlag flag)
    : _name(std::move(name)), _col_count(colCount), _row_count(rowCount), _layer_context(tilemap._layer_context), _size(tilemap._size), _tileset(tilemap._tileset),
      _position(std::move(position)), _scroller(std::move(scroller)), _flag(flag), _layer_tiles(_col_count * _row_count)
{
}

void TilemapLayer::render(RenderRequest& /*renderRequest*/, const V3& position)
{
    float width = _size->width();
    float height = _size->height();
    float tileWidth = static_cast<float>(_tileset->tileWidth()), tileHeight = static_cast<float>(_tileset->tileHeight());

    const bool isScrollable = _flag & Tilemap::LAYER_FLAG_SCROLLABLE;
    const V3 scroll = isScrollable ? _scroller->val() : V3();
    const V3 pos = _position->val();
    const Rect viewport = isScrollable ? Rect(scroll.x(), scroll.y(), std::max(scroll.x(), pos.x()) + width, std::max(scroll.y(), pos.y()) + height) :
                                         Rect(pos.x(), pos.y(), pos.x() + width, pos.y() + height);

    V3 selectionPosition;
    RectI selectionRange;
    if(getSelectionTileRange(viewport, selectionPosition, selectionRange))
    {
        const float ox = selectionPosition.x() + tileWidth / 2.0f - scroll.x();
        const float oy = selectionPosition.y() + tileHeight / 2.0f - scroll.y();
        renderTiles(position + V3(ox, oy, 0), selectionRange);
    }
}

bool TilemapLayer::getSelectionTileRange(const Rect& aabb, V3& selectionPosition, RectI& selectionRange) const
{
    float width = _size->width();
    float height = _size->height();
    float tileWidth = static_cast<float>(_tileset->tileWidth()), tileHeight = static_cast<float>(_tileset->tileHeight());
    const V3 pos = _position->val();

    float sx, ex, sy, ey;

    Rect intersection;
    const Rect aabbRel(aabb.left() - pos.x(), aabb.top() - pos.y(), aabb.right() - pos.x(), aabb.bottom() - pos.y());
    if(!aabbRel.intersect(Rect(0, 0, width, height), intersection))
        return false;
    Math::modBetween<float>(intersection.left(), intersection.right(), _tileset->tileWidth(), sx, ex);
    Math::modBetween<float>(intersection.top(), intersection.bottom(), _tileset->tileHeight(), sy, ey);

    int32_t rowStart = std::max(static_cast<int32_t>(sy / tileHeight), 0);
    int32_t colStart = std::max(static_cast<int32_t>(sx / tileWidth), 0);
    int32_t rowEnd = std::min<int32_t>(static_cast<int32_t>(_row_count), static_cast<int32_t>(ey / tileHeight));
    int32_t colEnd = std::min<int32_t>(static_cast<int32_t>(_col_count), static_cast<int32_t>(ex / tileWidth));

    selectionPosition = V3(sx, sy, 0) + pos;
    selectionRange = RectI(colStart, rowStart, colEnd, rowEnd);
    return true;
}

const String& TilemapLayer::name() const
{
    return _name;
}

const sp<Vec3>& TilemapLayer::position() const
{
    return _position;
}

void TilemapLayer::setPosition(const sp<Vec3>& position)
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

const sp<Vec3>& TilemapLayer::scroller() const
{
    return _scroller;
}

void TilemapLayer::setScroller(const sp<Vec3>& scroller)
{
    _scroller = scroller;
}

const sp<Tile>& TilemapLayer::getTile(uint32_t rowId, uint32_t colId) const
{
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    const LayerTile& layerTile = _layer_tiles.at(rowId * _col_count + colId);
    return layerTile.tile;
}

const sp<Tile>& TilemapLayer::getTileByPosition(float x, float y) const
{
    return getTile(static_cast<uint32_t>(y / _tileset->tileHeight()), static_cast<uint32_t>(x / _tileset->tileWidth()));
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, const sp<RenderObject>& renderObject)
{
    setTile(row, col, nullptr, renderObject);
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, int32_t tileId)
{
    if(tileId >= 0)
    {
        const sp<Tile>& tile = _tileset->getTile(tileId);
        DCHECK(tile, "TileId %d does not exist", tileId);
        setTile(row, col, tile);
    }
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, const sp<Tile>& tile)
{
    setTile(row, col, tile, nullptr);
}

void TilemapLayer::copyTiles(const std::vector<int32_t>& tiles, const RectI& dest)
{
    const int32_t destColCount = dest.width();
    for(int32_t i = dest.top(); i < dest.bottom(); ++i)
        for(int32_t j = dest.left(); j < dest.right(); ++j)
            setTile(i, j, tiles.at((i - dest.top()) * destColCount + j - dest.left()));
}

void TilemapLayer::reset()
{
    std::fill(_layer_tiles.begin(), _layer_tiles.end(), LayerTile());
}

void TilemapLayer::renderTiles(const V3& position, const RectI& renderRange)
{
    float tileWidth = static_cast<float>(_tileset->tileWidth());
    float tileHeight = static_cast<float>(_tileset->tileHeight());
    for(int32_t i = renderRange.top(); i < renderRange.bottom(); ++i)
    {
        float dy = (i - renderRange.top()) * tileHeight;
        for(int32_t j = renderRange.left(); j < renderRange.right(); ++j)
        {
            const LayerTile& tile = _layer_tiles.at(i * _col_count + j);
            if(tile.renderable)
                tile.renderable->requestUpdate(V3((j - renderRange.left()) * tileWidth, dy, 0) + position);
        }
    }
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, const sp<Tile>& tile, const sp<RenderObject>& renderObject)
{
    DCHECK(row < _row_count && col < _col_count, "Invaild tile position:(%d, %d), tilemap size(%d, %d)", row, col, _row_count, _col_count);
    uint32_t index = row * _col_count + col;
    const sp<RenderObject>& ro = renderObject ? renderObject : (tile ? tile->renderObject() : nullptr);
    sp<RenderablePassive> renderable = ro ? sp<RenderablePassive>::make(ro) : nullptr;
    if(renderable)
        _layer_context->add(renderable, sp<BooleanByWeakRef<Renderable>>::make(renderable, 1));
    _layer_tiles[index] = LayerTile(renderObject ? sp<Tile>::make(tile ? tile->id() : 0, tile ? tile->type() : 0, renderObject) : tile, std::move(renderable));
}

Tilemap::LayerFlag TilemapLayer::flag() const
{
    return _flag;
}

void TilemapLayer::setFlag(Tilemap::LayerFlag flag)
{
    _flag = flag;
}

TilemapLayer::LayerTile::LayerTile(sp<Tile> tile, sp<RenderablePassive> renderable)
    : tile(std::move(tile)), renderable(std::move(renderable)) {
}

}
