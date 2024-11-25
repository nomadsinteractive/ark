#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseAll : public BroadPhrase {
public:

    void create(CandidateIdType id, const V3& position, const V3& aabb) override;
    void update(CandidateIdType id, const V3& position, const V3& aabb) override;
    void remove(CandidateIdType id) override;

    Result search(const V3& position, const V3& size) override;
    Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

//  [[plugin::builder("broad-phrase-all")]]
    class BUILDER : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<BroadPhrase> build(const Scope& args) override;

    };

private:
    std::unordered_set<CandidateIdType> _candidates;
};

}
