#include "app/impl/broad_phrase/broad_phrase_tilemap.h"

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/inf/storage.h"
#include "core/util/log.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/components/shape.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"
#include "graphics/base/tile.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_filter.h"
#include "app/inf/broad_phrase_callback.h"
#include "app/inf/collider.h"
#include "app/inf/narrow_phrase.h"

namespace ark {

namespace {

BroadPhrase::CandidateIdType toCandidateId(const int32_t layerId, const int32_t row, const int32_t col)
{
    return layerId << 16 | ((row << 8) & 0xff00) | (col & 0xff);
}


BroadPhrase::Candidate makeCandidate(const BroadPhrase::CandidateIdType candidateId, const sp<Shape>& shape, const V2& position, sp<CollisionFilter> collisionFilter)
{
    return {candidateId, V3(position, 0), constants::QUATERNION_ONE, shape, std::move(collisionFilter)};
}

void addCandidate(BroadPhraseCallback& callback, const TilemapLayer& tilemapLayer, Set<int32_t>& candidateIdSet, Vector<BroadPhrase::Candidate>& candidates, int32_t row, int32_t col, int32_t layerId, const V2& tl, const V2& tileSize)
{
    if(row >= 0 && static_cast<uint32_t>(row) < tilemapLayer.rowCount() && col >= 0 && static_cast<uint32_t>(col) < tilemapLayer.colCount())
    {
        const int32_t candidateId = toCandidateId(layerId, row, col);
        if(!candidateIdSet.contains(candidateId))
        {
            const sp<Tile>& tile = tilemapLayer.getTile(static_cast<uint32_t>(col), static_cast<uint32_t>(row));
            candidateIdSet.insert(candidateId);
            if(tile)
                if(const int32_t shapeId = tile->shape()->type().hash(); shapeId != Shape::TYPE_NONE)
                {
                    callback.onStaticCandidate(candidateId, V3(tl + V2(col * tileSize.x(), row * tileSize.y()), 0), constants::QUATERNION_ONE, tile->shape(), tilemapLayer.collisionFilter());
                    candidates.push_back(makeCandidate(candidateId, tile->shape(), tl + V2(col * tileSize.x(), row * tileSize.y()), tilemapLayer.collisionFilter()));
                }
        }
    }
}
    
}

BroadPhraseTilemap::BroadPhraseTilemap(sp<Tilemap> tilemap)
    : _tilemap(std::move(tilemap))
{
}

sp<BroadPhrase::Coordinator> BroadPhraseTilemap::requestCoordinator()
{
    return nullptr;
}

BroadPhrase::Result BroadPhraseTilemap::search(BroadPhraseCallback& callback, const V3 position, const V3 size)
{
    Vector<Candidate> candidates;
    Set<int32_t> candidateIdSet;
    const V2 sizeHalf = size / V2(2.0, 2.0);
    const Rect aabb(V2(position) - sizeHalf, V2(position) + sizeHalf);
    const V2 tileSize(static_cast<float>(_tilemap->tileset()->tileWidth()), static_cast<float>(_tilemap->tileset()->tileHeight()));
    for(const TilemapLayer& i : _tilemap->layers())
        if(i.collisionFilter())
        {
            V3 selectionPoint;
            RectI selectionRange;
            int32_t layerId = static_cast<int32_t>(i.name().hash());
            if(i.getSelectionTileRange(aabb, selectionPoint, selectionRange))
                for(int32_t j = selectionRange.left(); j < selectionRange.right(); ++j)
                {
                    float px = selectionPoint.x() + (j - selectionRange.left()) * tileSize.x() + tileSize.x() / 2;
                    for(int32_t k = selectionRange.top(); k < selectionRange.bottom(); ++k)
                    {
                        const sp<Tile>& tile = i.getTile(j, k);
                        if(tile)
                        {
                            int32_t shapeId = tile->shape()->type().hash();
                            if(shapeId != Shape::TYPE_NONE)
                            {
                                int32_t candidateId = toCandidateId(layerId, k, j);
                                if(candidateIdSet.contains(candidateId))
                                {
                                    LOGW("Duplicated candidate found, this may be caused by duplidated layer name \"%s\"", i.name().c_str());
                                    continue;
                                }
                                candidateIdSet.insert(candidateId);
                                candidates.push_back(makeCandidate(candidateId, tile->shape(), V2(px, selectionPoint.y() + (k - selectionRange.top()) * tileSize.y() + tileSize.y() / 2), i.collisionFilter()));
                                callback.onStaticCandidate(candidateId, V3(px, selectionPoint.y() + (k - selectionRange.top()) * tileSize.y() + tileSize.y() / 2, 0), constants::QUATERNION_ONE, tile->shape(), i.collisionFilter());
                            }
                        }
                    }
                }
        }
    return BroadPhrase::Result({}, std::move(candidates));
}

BroadPhrase::Result BroadPhraseTilemap::rayCast(BroadPhraseCallback& callback, const V3 from, const V3 to, const sp<CollisionFilter>& collisionFilter)
{
    Vector<Candidate> candidates;
    float tileWidth = _tilemap->tileset()->tileWidth();
    float tileHeight = _tilemap->tileset()->tileHeight();
    const Rect aabb = Rect(from, to);
    for(const TilemapLayer& i : _tilemap->layers())
        if(i.collisionFilter() && (!collisionFilter || i.collisionFilter()->collisionTest(*collisionFilter)))
        {
            Rect intersection;
            float tilemapLayerWidth = tileWidth * static_cast<float>(i.colCount());
            float tilemapLayerHeight = tileHeight * static_cast<float>(i.rowCount());
            const V2 tilemapPosition = i.position().val();
            const Rect tilemapLayerAabb(tilemapPosition, tilemapPosition + V2(tilemapLayerWidth, tilemapLayerHeight));
            if(tilemapLayerAabb.intersect(aabb, intersection))
            {
                const int32_t layerId = static_cast<int32_t>(i.name().hash());
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
                Set<int32_t> candidateIdSet;
                for(uint32_t j = 0; j <= step; ++j)
                {
                    const V2 pos = start + delta * static_cast<float>(j);
                    const int32_t row = static_cast<int32_t>((pos.y() - tilemapLayerAabb.top()) / tileHeight);
                    const int32_t col = static_cast<int32_t>((pos.x() - tilemapLayerAabb.left()) / tileWidth);
                    addCandidate(callback, i, candidateIdSet, candidates, row, col, layerId, tl, tileSize);
                    if(j != 0)
                    {
                        if(xwise)
                            addCandidate(callback, i, candidateIdSet, candidates, row, col + pre, layerId, tl, tileSize);
                        else
                            addCandidate(callback, i, candidateIdSet, candidates, row + pre, col, layerId, tl, tileSize);
                    }
                }
            }
        }
    return Result({}, std::move(candidates));
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
