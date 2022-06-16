#ifndef ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_TILEMAP_H_
#define ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_TILEMAP_H_

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseTilemap : public BroadPhrase {
public:
    BroadPhraseTilemap(sp<Tilemap> tilemap, NarrowPhrase& narrowPhrase);

    virtual void create(int32_t id, const V3& position, const V3& aabb) override;
    virtual void update(int32_t id, const V3& position, const V3& aabb) override;
    virtual void remove(int32_t id) override;

    virtual Result search(const V3& position, const V3& size) override;
    virtual Result rayCast(const V3& from, const V3& to) override;

//  [[plugin::builder("broad-phrase-tilemap")]]
    class BUILDER : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<BroadPhrase> build(const Scope& args) override;

    private:
        sp<Builder<Tilemap>> _tilemap;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
    };

private:
    int32_t toCandidateId(int32_t layerId, int32_t row, int32_t col) const;
    int32_t toCandidateShapeId(const sp<RenderObject>& renderObject, int32_t row, int32_t col) const;

    void addCandidate(const TilemapLayer& tilemapLayer, std::set<int32_t>& candidateIdSet, std::vector<BroadPhrase::Candidate>& candidates, int32_t row, int32_t col, int32_t layerId, const V2& tl, const V2& tileSize);
    Candidate makeCandidate(int32_t candidateId, int32_t shapeId, const V2& position) const;

private:
    sp<Tilemap> _tilemap;
    std::vector<Box> _tile_shapes;
};

}

#endif
