#include "app/impl/broad_phrase/broad_phrase_tilemap.h"

#include "core/base/bean_factory.h"
#include "core/inf/storage.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"
#include "graphics/base/tile.h"

#include "renderer/base/resource_loader_context.h"

#include "app/inf/collider.h"

namespace ark {

BroadPhraseTilemap::BroadPhraseTilemap(sp<Tilemap> tilemap)
    : _tilemap(std::move(tilemap))
{
}

sp<Vec3> BroadPhraseTilemap::create(int32_t id, const sp<Vec3>& position, const sp<Vec3>& size)
{
    return position;
}

void BroadPhraseTilemap::remove(int32_t /*id*/)
{
}

BroadPhrase::Result BroadPhraseTilemap::search(const V3& position, const V3& size)
{
    std::vector<Candidate> candidates;
    const V2 sizeHalf = size / V2(2.0, 2.0);
    const Rect aabb(V2(position) - sizeHalf, V2(position) + sizeHalf);
    float tileWidth = static_cast<float>(_tilemap->tileset()->tileWidth());
    float tileHeight = static_cast<float>(_tilemap->tileset()->tileHeight());
    for(const sp<TilemapLayer>& i : _tilemap->layers())
        if(i->flag() & Tilemap::LAYER_FLAG_COLLIDABLE)
        {
            V3 selectionPoint;
            RectI selectionRange;
            int32_t layerId = static_cast<int32_t>(i->name().hash());
            if(i->getSelectionTileRange(aabb, selectionPoint, selectionRange))
                for(int32_t j = selectionRange.left(); j < selectionRange.right(); ++j)
                {
                    float px = selectionPoint.x() + (j - selectionRange.left()) * tileWidth + tileWidth / 2;
                    for(int32_t k = selectionRange.top(); k < selectionRange.bottom(); ++k)
                    {
                        const sp<Tile>& tile = i->getTile(k, j);
                        if(tile)
                        {
                            int32_t shapeId = tile->type();
                            if(shapeId != Collider::BODY_SHAPE_NONE)
                                candidates.emplace_back(toCandidateId(layerId, k, j), V2(px, selectionPoint.y() + (k - selectionRange.top()) * tileHeight + tileHeight / 2), 0, shapeId);
                        }
                    }
                }
        }
    return BroadPhrase::Result({}, std::move(candidates));
}

BroadPhrase::Result BroadPhraseTilemap::rayCast(const V3& from, const V3& to)
{
    std::vector<Candidate> candidates;
    float tileWidth = static_cast<float>(_tilemap->tileset()->tileWidth());
    float tileHeight = static_cast<float>(_tilemap->tileset()->tileHeight());
    const Rect aabb = Rect(from, to);
    for(const sp<TilemapLayer>& i : _tilemap->layers())
        if(i->flag() & Tilemap::LAYER_FLAG_COLLIDABLE)
        {
            Rect intersection;
            float tilemapLayerWidth = tileWidth * static_cast<float>(i->colCount());
            float tilemapLayerHeight = tileHeight * static_cast<float>(i->rowCount());
            const V2 tilemapPosition = i->position()->val();
            const Rect tilemapLayerAabb(tilemapPosition, tilemapPosition + V2(tilemapLayerWidth, tilemapLayerHeight));
            if(tilemapLayerAabb.intersect(aabb, intersection))
            {
                const int32_t layerId = static_cast<int32_t>(i->name().hash());
                const float rayIntersectionWidth = intersection.right() - intersection.left();
                const float rayIntersectionHeight = intersection.bottom() - intersection.top();
                const bool xwise = aabb.width() > aabb.height();
                const uint32_t step = static_cast<uint32_t>(xwise ? std::abs(rayIntersectionWidth / tileWidth) : std::abs(rayIntersectionHeight / tileHeight));
                const float dx = xwise ? (rayIntersectionWidth > 0 ? tileWidth : -tileWidth) : (rayIntersectionWidth / static_cast<float>(step));
                const float dy = xwise ? (rayIntersectionHeight / static_cast<float>(step)) : (rayIntersectionHeight > 0 ? tileHeight : -tileHeight);
                const float st = xwise ? dx / aabb.width() : dy / aabb.height();
                const float ot = xwise ? (intersection.left() - aabb.left()) / aabb.width() : (intersection.top() - aabb.top()) / aabb.height();

                const V2 start(Math::lerp(aabb.left(), aabb.right(), ot), Math::lerp(aabb.top(), aabb.bottom(), ot));
                const V2 delta(Math::lerp(0, aabb.right() - aabb.left(), std::abs(st)), Math::lerp(0, aabb.bottom() - aabb.top(), std::abs(st)));
                const V2 tileSize(tileWidth, tileHeight);
                const V2 tl = V2(tilemapLayerAabb.left(), tilemapLayerAabb.top()) + tileSize / 2.0f;
                const int32_t pre = st > 0 ? -1 : 1;
                std::set<int32_t> candidateIdSet;
                for(uint32_t j = 0; j <= step; ++j)
                {
                    const V2 pos = start + delta * static_cast<float>(j);
                    const int32_t row = static_cast<int32_t>((pos.y() - tilemapLayerAabb.top()) / tileHeight);
                    const int32_t col = static_cast<int32_t>((pos.x() - tilemapLayerAabb.left()) / tileWidth);
                    addCandidate(i, candidateIdSet, candidates, row, col, layerId, tl, tileSize);
                    if(j != 0)
                    {
                        if(xwise)
                            addCandidate(i, candidateIdSet, candidates, row, col + pre, layerId, tl, tileSize);
                        else
                            addCandidate(i, candidateIdSet, candidates, row + pre, col, layerId, tl, tileSize);
                    }
                }
            }
        }
    return BroadPhrase::Result({}, std::move(candidates));
}

int32_t BroadPhraseTilemap::toCandidateId(int32_t layerId, int32_t row, int32_t col) const
{
    return layerId << 16 | ((row << 8) & 0xff00) | (col & 0xff);
}

void BroadPhraseTilemap::addCandidate(const TilemapLayer& tilemapLayer, std::set<int32_t>& candidateIdSet, std::vector<Candidate>& candidates, int32_t row, int32_t col, int32_t layerId, const V2& tl, const V2& tileSize)
{
    if(row >= 0 && static_cast<uint32_t>(row) < tilemapLayer.rowCount() && col >= 0 && static_cast<uint32_t>(col) < tilemapLayer.colCount())
    {
        const int32_t candidateId = toCandidateId(layerId, static_cast<int32_t>(row), static_cast<int32_t>(col));
        if(candidateIdSet.find(candidateId) == candidateIdSet.end())
        {
            const sp<Tile>& tile = tilemapLayer.getTile(row, col);
            candidateIdSet.insert(candidateId);
            if(tile)
            {
                int32_t shapeId = tile->type();
                if(shapeId != Collider::BODY_SHAPE_NONE)
                    candidates.emplace_back(candidateId, tl + V2(col * tileSize.x(), row * tileSize.y()), 0, shapeId);
            }
        }
    }
}

BroadPhraseTilemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _tilemap(factory.ensureBuilder<Tilemap>(manifest, "tilemap"))
{
}

sp<BroadPhrase> BroadPhraseTilemap::BUILDER::build(const Scope& args)
{
    return sp<BroadPhraseTilemap>::make(_tilemap->build(args));
}

}