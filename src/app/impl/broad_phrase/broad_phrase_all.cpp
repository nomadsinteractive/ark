#include "app/impl/broad_phrase/broad_phrase_all.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"

#include "app/inf/broad_phrase.h"
#include "app/inf/broad_phrase_callback.h"

namespace ark {

class BroadPhraseAll::Stub final : public BroadPhrase::Coordinator {
public:
    void create(const RefId id, const V3& position, const V3& aabb) override
    {
        _candidates.insert(id);
    }

    void update(RefId id, const V3& position, const V3& aabb) override
    {
    }

    void remove(const RefId id) override
    {
        const auto iter = _candidates.find(id);
        CHECK(iter != _candidates.end(), "RigidBody(%uz) not registered", id);
        _candidates.erase(iter);
    }

    HashSet<RefId> _candidates;
};

BroadPhraseAll::BroadPhraseAll()
    : _stub(sp<Stub>::make())
{
}

void BroadPhraseAll::search(BroadPhraseCallback& callback, const V3 /*position*/, const V3 /*size*/)
{
    for(const auto i : _stub->_candidates)
        callback.onRigidbodyCandidate(i);
}

void BroadPhraseAll::rayCast(BroadPhraseCallback& callback, const V3 from, const V3 to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    search(callback, from, to);
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
