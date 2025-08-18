#pragma once

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

    void create(CandidateIdType id, const V3& position, const V3& aabb) override;
    void update(CandidateIdType id, const V3& position, const V3& aabb) override;
    void remove(CandidateIdType id) override;

    Result search(const V3& position, const V3& size) override;
    Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

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
    CandidateIdType toCandidateId(int32_t layerId, int32_t row, int32_t col) const;

    void addCandidate(const TilemapLayer& tilemapLayer, std::set<int32_t>& candidateIdSet, std::vector<BroadPhrase::Candidate>& candidates, int32_t row, int32_t col, int32_t layerId, const V2& tl, const V2& tileSize);
    Candidate makeCandidate(CandidateIdType candidateId, const sp<Shape>& shape, const V2& position, sp<CollisionFilter> collisionFilter) const;

private:
    sp<Tilemap> _tilemap;
};

}
