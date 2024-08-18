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

#include "renderer/base/shader.h"
#include "renderer/base/model.h"

namespace ark {

TilemapLayer::TilemapLayer(sp<Tileset> tileset, String name, uint32_t colCount, uint32_t rowCount, sp<Vec3> position, sp<Boolean> visible, sp<CollisionFilter> collisionFilter)
    : _name(std::move(name)), _col_count(colCount), _row_count(rowCount), _size(sp<Size>::make(tileset->tileWidth() * colCount, tileset->tileHeight() * rowCount)), _visible(std::move(visible), true),
      _collision_filter(std::move(collisionFilter)), _stub(sp<Stub>::make(colCount, rowCount, std::move(tileset), SafeVar<Vec3>(std::move(position)), 0)), _layer_tiles(colCount * rowCount)
{
}

bool TilemapLayer::getSelectionTileRange(const Rect& aabb, V3& selectionPosition, RectI& selectionRange) const
{
    float width = _size->widthAsFloat();
    float height = _size->heightAsFloat();
    const V3 pos = _stub->_position.val();
    float tileWidth = _stub->_tileset->tileWidth(), tileHeight = _stub->_tileset->tileHeight();

    float sx, ex, sy, ey;

    Rect intersection;
    const Rect aabbRel(aabb.left() - pos.x(), aabb.top() - pos.y(), aabb.right() - pos.x(), aabb.bottom() - pos.y());
    if(!aabbRel.intersect(Rect(0, 0, width, height), intersection))
        return false;
    Math::modBetween<float>(intersection.left(), intersection.right(), tileWidth, sx, ex);
    Math::modBetween<float>(intersection.top(), intersection.bottom(), tileHeight, sy, ey);

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

const SafeVar<Vec3>& TilemapLayer::position() const
{
    return _stub->_position;
}

void TilemapLayer::setPosition(sp<Vec3> position)
{
    _stub->_position.reset(std::move(position));
    if(_layer_context)
        _layer_context->markDirty();

}

float TilemapLayer::zorder() const
{
    return _stub->_zorder;
}

const sp<Tileset>& TilemapLayer::tileset() const
{
    return _stub->_tileset;
}

uint32_t TilemapLayer::colCount() const
{
    return _col_count;
}

uint32_t TilemapLayer::rowCount() const
{
    return _row_count;
}

const SafeVar<Boolean>& TilemapLayer::visible() const
{
    return _visible;
}

void TilemapLayer::setVisible(sp<Boolean> visible)
{
    _visible.reset(std::move(visible));
}

sp<Tile> TilemapLayer::getTile(uint32_t colId, uint32_t rowId) const
{
    CHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", colId, rowId, _row_count, _col_count);
    const sp<RenderableTile>& layerTile = _layer_tiles.at(rowId * _col_count + colId);
    return layerTile ? layerTile->_tile : nullptr;
}

std::vector<int32_t> TilemapLayer::getTileRect(const RectI& rect) const
{
    const RectI srcRect = rect.upright();
    ASSERT(srcRect.top() >= 0 && srcRect.left() >= 0);
    CHECK(srcRect.right() <= _col_count && srcRect.bottom() <= _row_count, "Rect out of layer bounds: rect.right: %d, colCount: %d, rect.bottom: %d, rowCount: %d", rect.right(), _col_count, rect.bottom(), _row_count);
    std::vector<int32_t> tiles(static_cast<size_t>(srcRect.width() * srcRect.height()));
    for(int32_t i = srcRect.top(); i < srcRect.bottom(); ++i)
        for(int32_t j = srcRect.left(); j < srcRect.right(); ++j)
        {
            const sp<RenderableTile>& rt = _layer_tiles.at(i * _col_count + j);
            size_t idx = static_cast<size_t>((i - srcRect.top()) * srcRect.width() + j - srcRect.left());
            if(!rt || !rt->_tile)
                tiles[idx] = -1;
            else
                tiles[idx] = rt->_tile->id();
        }
    return tiles;
}

sp<Tile> TilemapLayer::getTileByPosition(float x, float y) const
{
    return getTile(static_cast<uint32_t>(x / _stub->_tileset->tileWidth()), static_cast<uint32_t>(y / _stub->_tileset->tileHeight()));
}

void TilemapLayer::setTile(uint32_t col, uint32_t row, const sp<RenderObject>& renderObject)
{
    setTile(col, row, nullptr, renderObject);
}

void TilemapLayer::setTile(uint32_t col, uint32_t row, int32_t tileId)
{
    if(tileId >= 0)
    {
        const sp<Tile>& tile = _stub->_tileset->getTile(tileId);
        CHECK(tile, "TileId %d does not exist", tileId);
        setTile(col, row, tile);
    }
    else
    {
        CHECK(tileId == -1, "Illegal negative tileId: %d", tileId);
        setTile(col, row, nullptr, nullptr);
    }
}

void TilemapLayer::setTile(uint32_t col, uint32_t row, const sp<Tile>& tile)
{
    CHECK_WARN(tile == nullptr || tile->renderObject(), "This tile has no RenderObject assigned");
    setTile(col, row, tile, nullptr);
}

void TilemapLayer::setTileRect(const std::vector<int32_t>& tiles, const RectI& dest)
{
    const int32_t destColCount = dest.width();
    for(int32_t i = dest.top(); i < dest.bottom(); ++i)
        for(int32_t j = dest.left(); j < dest.right(); ++j)
            setTile(j, i, tiles.at(static_cast<size_t>((i - dest.top()) * destColCount + j - dest.left())));
}

void TilemapLayer::resize(uint32_t colCount, uint32_t rowCount, uint32_t offsetX, uint32_t offsetY)
{
    CHECK(colCount >= _col_count && rowCount >= _row_count, "Changing size(%d, %d) to (%d, %d) failed. TilemapLayer can not be shrinked.", _col_count, _row_count, colCount, rowCount);
    CHECK(offsetX <= (colCount - _col_count) && offsetY <= (rowCount - _row_count), "Offset position out of bounds(%d, %d)", offsetX, offsetY);
    const bool positionChanged = offsetX || offsetY;
    float tileWidth = _stub->_tileset->tileWidth(), tileHeight = _stub->_tileset->tileWidth();
    Stub stub(colCount, rowCount, std::move(_stub->_tileset), std::move(_stub->_position), _stub->_zorder);

    std::vector<sp<RenderableTile>> layerTiles(colCount * rowCount);
    for(uint32_t i = 0; i < _row_count; ++i)
        for(uint32_t j = 0; j < _col_count; ++j)
        {
            sp<RenderableTile>& renderTile = layerTiles[(i + offsetY) * colCount + j + offsetX];
            renderTile = std::move(_layer_tiles[i * _col_count + j]);
            if(renderTile && positionChanged)
            {
                float dx = static_cast<float>(j + offsetX) * tileWidth + tileWidth / 2;
                float dy = static_cast<float>(i + offsetY) * tileHeight + tileHeight / 2;
                renderTile->_position = V3(dx, dy, 0);
            }
        }

    *_stub = std::move(stub);
    _col_count = colCount;
    _row_count = rowCount;
    _size->setWidth(_stub->_tileset->tileWidth() * colCount);
    _size->setHeight(_stub->_tileset->tileHeight() * rowCount);
    _layer_tiles = std::move(layerTiles);
    if(_layer_context && positionChanged)
        _layer_context->markDirty();
}

void TilemapLayer::clear()
{
    std::fill(_layer_tiles.begin(), _layer_tiles.end(), nullptr);
}

void TilemapLayer::foreachTile(const std::function<bool (uint32_t, uint32_t, const sp<Tile>&)>& callback) const
{
    for(uint32_t i = 0; i < _row_count; ++i)
        for(uint32_t j = 0; j < _col_count; ++j)
        {
            const sp<RenderableTile>& layerTile = _layer_tiles.at(i * _col_count + j);
            if(layerTile && layerTile->_tile && !callback(j, i, layerTile->_tile))
                return;
        }
}

void TilemapLayer::setTile(uint32_t col, uint32_t row, const sp<Tile>& tile, const sp<RenderObject>& renderObject)
{
    CHECK(row < _row_count && col < _col_count, "Invaild tile position:(%d, %d), tilemap size(%d, %d)", row, col, _row_count, _col_count);
    uint32_t index = row * _col_count + col;
    const sp<RenderObject>& ro = renderObject ? renderObject : (tile ? tile->renderObject() : nullptr);
    sp<Tile> tileDup = renderObject ? (tile ? sp<Tile>::make(tile->id(), tile->type(), tile->shapeId(), renderObject) : sp<Tile>::make(0, "", -1, renderObject)) : tile;
    float tileWidth = _stub->_tileset->tileWidth(), tileHeight = _stub->_tileset->tileWidth();
    float dx = static_cast<float>(col) * tileWidth + tileWidth / 2;
    float dy = static_cast<float>(row) * tileHeight + tileHeight / 2;
    sp<RenderableTile> renderableTile = ro ? sp<RenderableTile>::make(_stub, std::move(tileDup), ro, V3(dx, dy, 0.0)) : nullptr;
    sp<RenderableTile>& targetTile = _layer_tiles[index];
    if(targetTile)
        targetTile->dispose();
    if(_layer_context && renderableTile)
        _layer_context->add(renderableTile);
    targetTile = std::move(renderableTile);
}

const sp<CollisionFilter>& TilemapLayer::collisionFilter() const
{
    return _collision_filter;
}

void TilemapLayer::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _collision_filter = std::move(collisionFilter);
}

const sp<LayerContext>& TilemapLayer::layerContext() const
{
    return _layer_context;
}

void TilemapLayer::setLayerContext(sp<LayerContext> layerContext)
{
    _layer_context = std::move(layerContext);
    for(const sp<RenderableTile>& i : _layer_tiles)
        if(i)
            _layer_context->add(i);
}

TilemapLayer::RenderableTile::RenderableTile(const sp<Stub>& stub, sp<Tile> tile, sp<RenderObject> renderable, const V3& position)
    : _stub(stub), _tile(std::move(tile)), _renderable(std::move(renderable)), _position(position)
{
    ASSERT(_renderable);
}

Renderable::StateBits TilemapLayer::RenderableTile::updateState(const RenderRequest& renderRequest)
{
    return _renderable ? _renderable->updateState(renderRequest) : Renderable::RENDERABLE_STATE_DISCARDED;
}

Renderable::Snapshot TilemapLayer::RenderableTile::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    if(!_renderable)
        return {RENDERABLE_STATE_NONE};

    const V3 tileTranslate = _position + V3(0, 0, _stub->_zorder);
    Snapshot snapshot = _renderable->snapshot(snapshotContext, renderRequest, state);
    snapshot._position += tileTranslate;
    return snapshot;
}

void TilemapLayer::RenderableTile::dispose()
{
    _renderable = nullptr;
}

TilemapLayer::Stub::Stub(size_t colCount, size_t rowCount, sp<Tileset> tileset, SafeVar<Vec3> position, float zorder)
    : _col_count(colCount), _row_count(rowCount), _tileset(std::move(tileset)), _position(std::move(position)), _zorder(zorder)
{
}

}
