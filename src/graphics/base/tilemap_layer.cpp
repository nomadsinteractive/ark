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

TilemapLayer::TilemapLayer(sp<Tileset> tileset, String name, uint32_t rowCount, uint32_t colCount, sp<Vec3> position, sp<Vec3> scroller, sp<Boolean> visible, Tilemap::LayerFlag flag)
    : _name(std::move(name)), _col_count(colCount), _row_count(rowCount), _size(sp<Size>::make(tileset->tileWidth() * colCount, tileset->tileHeight() * rowCount)),
      _scroller(std::move(scroller)), _visible(std::move(visible)), _zorder(0), _flag(flag), _renderable_batch(sp<BatchImpl>::make(colCount, rowCount, std::move(tileset), std::move(position)))
{
}

bool TilemapLayer::getSelectionTileRange(const Rect& aabb, V3& selectionPosition, RectI& selectionRange) const
{
    float width = _size->widthAsFloat();
    float height = _size->heightAsFloat();
    const V3 pos = _renderable_batch->_position.val();
    float tileWidth = _renderable_batch->_tileset->tileWidth(), tileHeight = _renderable_batch->_tileset->tileHeight();

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

sp<Vec3> TilemapLayer::position()
{
    return _renderable_batch->_position.ensure();
}

void TilemapLayer::setPosition(sp<Vec3> position)
{
    _renderable_batch->_position.reset(std::move(position));
}

float TilemapLayer::zorder() const
{
    return _zorder;
}

const sp<Tileset>& TilemapLayer::tileset() const
{
    return _renderable_batch->_tileset;
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

void TilemapLayer::setScroller(sp<Vec3> scroller)
{
    _scroller = std::move(scroller);
}

const sp<Tile>& TilemapLayer::getTile(uint32_t rowId, uint32_t colId) const
{
    CHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    const LayerTile& layerTile = _renderable_batch->_layer_tiles.at(rowId * _col_count + colId);
    return layerTile.tile;
}

std::vector<int32_t> TilemapLayer::getTileRect(const RectI& rect) const
{
    const RectI srcRect = rect.upright();
    DASSERT(srcRect.top() >= 0 && srcRect.left() >= 0);
    CHECK(srcRect.right() <= _col_count && srcRect.bottom() <= _row_count, "Rect out of layer bounds: rect.right: %d, colCount: %d, rect.bottom: %d, rowCount: %d", rect.right(), _col_count, rect.bottom(), _row_count);
    std::vector<int32_t> tiles(static_cast<size_t>(srcRect.width() * srcRect.height()));
    for(int32_t i = srcRect.top(); i < srcRect.bottom(); ++i)
        for(int32_t j = srcRect.left(); j < srcRect.right(); ++j)
        {
            const sp<Tile>& tile = _renderable_batch->_layer_tiles.at(i * _col_count + j).tile;
            tiles[static_cast<size_t>((i - srcRect.top()) * srcRect.width() + j - srcRect.left())] = tile ? tile->id() : -1;
        }
    return tiles;
}

const sp<Tile>& TilemapLayer::getTileByPosition(float x, float y) const
{
    return getTile(static_cast<uint32_t>(y / _renderable_batch->_tileset->tileHeight()), static_cast<uint32_t>(x / _renderable_batch->_tileset->tileWidth()));
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, const sp<RenderObject>& renderObject)
{
    setTile(row, col, nullptr, renderObject);
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, int32_t tileId)
{
    if(tileId >= 0)
    {
        const sp<Tile>& tile = _renderable_batch->_tileset->getTile(tileId);
        CHECK(tile, "TileId %d does not exist", tileId);
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
            setTile(i, j, tiles.at(static_cast<size_t>((i - dest.top()) * destColCount + j - dest.left())));
}

void TilemapLayer::reset()
{
    std::fill(_renderable_batch->_layer_tiles.begin(), _renderable_batch->_layer_tiles.end(), LayerTile());
}

void TilemapLayer::foreachTile(const std::function<bool (uint32_t, uint32_t, const sp<Tile>&)>& callback) const
{
    for(uint32_t i = 0; i < _row_count; ++i)
        for(uint32_t j = 0; j < _col_count; ++j)
        {
            const LayerTile& layerTile = _renderable_batch->_layer_tiles.at(i * _col_count + j);
            if(layerTile.tile && !callback(j, i, layerTile.tile))
                return;
        }
}

void TilemapLayer::setTile(uint32_t row, uint32_t col, const sp<Tile>& tile, const sp<RenderObject>& renderObject)
{
    DCHECK(row < _row_count && col < _col_count, "Invaild tile position:(%d, %d), tilemap size(%d, %d)", row, col, _row_count, _col_count);
    uint32_t index = row * _col_count + col;
    const sp<RenderObject>& ro = renderObject ? renderObject : (tile ? tile->renderObject() : nullptr);
    sp<Tile> tileDup = renderObject ? (tile ? sp<Tile>::make(tile->id(), tile->type(), tile->shapeId(), renderObject) : sp<Tile>::make(0, "", -1, renderObject)) : tile;
    _renderable_batch->_layer_tiles[index] = LayerTile(std::move(tileDup), ro);
}

Tilemap::LayerFlag TilemapLayer::flag() const
{
    return _flag;
}

void TilemapLayer::setFlag(Tilemap::LayerFlag flag)
{
    _flag = flag;
}

TilemapLayer::LayerTile::LayerTile(sp<Tile> tile, sp<RenderObject> renderable)
    : tile(std::move(tile)), renderable(std::move(renderable)) {
}

TilemapLayer::BatchImpl::BatchImpl(size_t colCount, size_t rowCount, sp<Tileset> tileset, sp<Vec3> position)
    : _col_count(colCount), _row_count(rowCount), _tileset(std::move(tileset)), _position(position), _layer_tiles(colCount * rowCount)
{
}

bool TilemapLayer::BatchImpl::preSnapshot(const RenderRequest& renderRequest, LayerContext& lc)
{
    bool needsReload = false;
    for(LayerTile& i : _layer_tiles)
    {
        if(i.renderable)
        {
            i.state = i.renderable->updateState(renderRequest);
            if(i.state.hasState(Renderable::RENDERABLE_STATE_DISPOSED))
            {
                needsReload = true;
                i.renderable = nullptr;
            }
        }
    }
    return needsReload;
}

void TilemapLayer::BatchImpl::snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output)
{
    const PipelineInput& pipelineInput = output.pipelineInput();
    const bool visible = lc._visible.val();
    const bool needsReload = lc._position_changed || lc._render_done != visible || output.needsReload();
    const bool hasDefaultVaryings = static_cast<bool>(lc._varyings);
    const Varyings::Snapshot defaultVaryingsSnapshot = hasDefaultVaryings ? lc._varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();

    float tileWidth = _tileset->tileWidth(), tileHeight = _tileset->tileWidth();
    const V3 posOff = lc._position + _position.val();

    for(size_t i = 0; i < _row_count; ++i)
    {
        size_t rowIndex = i * _col_count;
        float dy = static_cast<float>(i) * tileHeight + tileHeight / 2;
        for(size_t j = 0; j < _col_count; ++j)
        {
            LayerTile& tile = _layer_tiles.at(rowIndex + j);
            if(tile.renderable)
            {
                Renderable::State state = tile.state;
                if(needsReload)
                    state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
                if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
                    state.setState(Renderable::RENDERABLE_STATE_VISIBLE, visible);
                Renderable::Snapshot snapshot = tile.renderable->snapshot(pipelineInput, renderRequest, posOff + V3(j * tileWidth + tileWidth / 2, dy, 0), state.stateBits());
                if(hasDefaultVaryings && !snapshot._varyings)
                    snapshot._varyings = defaultVaryingsSnapshot;
                output.addSnapshot(lc, std::move(snapshot));
            }
        }
    }
}

}
