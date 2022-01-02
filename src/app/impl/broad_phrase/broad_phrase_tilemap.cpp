#include "app/impl/broad_phrase/broad_phrase_tilemap.h"

#include "core/base/bean_factory.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"

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
    std::map<int32_t, Candidate> candidates;
    const V2 sizeHalf = size / V2(2.0, 2.0);
    const Rect aabb(V2(position) - sizeHalf, V2(position) + sizeHalf);
    float tileWidth = _tilemap->tileset()->tileWidth();
    float tileHeight = _tilemap->tileset()->tileHeight();
    for(const sp<TilemapLayer>& i : _tilemap->layers())
    {
        if(i->flag() & Tilemap::LAYER_FLAG_COLLIDABLE)
        {
            int32_t layerId = static_cast<int32_t>(i->name().hash());
            V3 selectionPoint;
            RectI selectionRange;
            if(i->getSelectionTileRange(aabb, selectionPoint, selectionRange))
            {
                for(int32_t j = selectionRange.left(); j < selectionRange.right(); ++j)
                {
                    float px = selectionPoint.x() + (j - selectionRange.left()) * tileWidth + tileWidth / 2;
                    for(int32_t k = selectionRange.top(); k < selectionRange.bottom(); ++k)
                    {
                        const sp<RenderObject>& renderObject = i->getTile(k, j);
                        if(renderObject)
                        {
                            Candidate candidate(V2(px, selectionPoint.y() + (k - selectionRange.top()) * tileHeight + tileHeight / 2), 0, toCandidateShapeId(renderObject, k, j));
                            candidates.emplace(std::make_pair(toCandidateId(layerId, k, j), candidate));
                        }
                    }
                }
            }
        }
    }
    return BroadPhrase::Result({}, std::move(candidates));
}

BroadPhrase::Result BroadPhraseTilemap::rayCast(const V3& from, const V3& to)
{
    return BroadPhrase::Result({}, {});
}

int32_t BroadPhraseTilemap::toCandidateId(int32_t layerId, int32_t row, int32_t col) const
{
    return layerId << 16 | ((row << 8) & 0xff00) | (col & 0xff);
}

int32_t BroadPhraseTilemap::toCandidateShapeId(const sp<RenderObject>& renderObject, int32_t row, int32_t col) const
{
    const int32_t renderObjectId = renderObject->type()->val();
    const auto iter = _shape_id_mappings.find(renderObjectId);
    return iter != _shape_id_mappings.end() ? iter->second : Collider::BODY_SHAPE_AABB;
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
