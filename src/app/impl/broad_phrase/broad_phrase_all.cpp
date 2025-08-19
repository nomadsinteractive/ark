#include "app/impl/broad_phrase/broad_phrase_all.h"

#include "app/inf/broad_phrase.h"
#include "core/base/bean_factory.h"
#include "core/util/math.h"

namespace ark {

class BroadPhraseAll::Stub final : public BroadPhrase::Coordinator {
public:
    void create(const CandidateIdType id, const V3& position, const V3& aabb) override
    {
        _candidates.insert(id);
    }

    void update(CandidateIdType id, const V3& position, const V3& aabb) override
    {
    }

    void remove(const CandidateIdType id) override
    {
        const auto iter = _candidates.find(id);
        CHECK(iter != _candidates.end(), "RigidBody(%uz) not registered", id);
        _candidates.erase(iter);
    }

    HashSet<CandidateIdType> _candidates;
};

BroadPhraseAll::BroadPhraseAll()
    : _stub(sp<Stub>::make())
{
}

BroadPhrase::Result BroadPhraseAll::search(const V3& /*position*/, const V3& /*size*/)
{
    Result result;
    result._dynamic_candidates = _stub->_candidates;
    return result;
}

BroadPhrase::Result BroadPhraseAll::rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    return search(from, to);
}

sp<BroadPhrase::Coordinator> BroadPhraseAll::requestCoordinator()
{
    return _stub.cast<Coordinator>();
}

sp<BroadPhrase> BroadPhraseAll::BUILDER::build(const Scope& /*args*/)
{
    return sp<BroadPhrase>::make<BroadPhraseAll>();
}

}
