#include "app/impl/broad_phrase/broad_phrase_all.h"

#include <algorithm>

#include "core/base/bean_factory.h"
#include "core/util/math.h"
#include "core/util/log.h"

namespace ark {

BroadPhraseAll::BroadPhraseAll()
{
}

void BroadPhraseAll::create(int32_t id, const V3& /*position*/, const V3& /*aabb*/)
{
    _candidates.insert(id);
}

void BroadPhraseAll::update(int32_t /*id*/, const V3& /*position*/, const V3& /*aabb*/)
{
}

void BroadPhraseAll::remove(int32_t id)
{
    const auto iter = _candidates.find(id);
    DCHECK(iter != _candidates.end(), "RigidBody(%d) not registered", id);
    _candidates.erase(iter);
}

BroadPhrase::Result BroadPhraseAll::search(const V3& /*position*/, const V3& /*size*/)
{
    BroadPhrase::Result result;
    result._dynamic_candidates = _candidates;
    return result;
}

BroadPhrase::Result BroadPhraseAll::rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    return search(from, to);
}

BroadPhraseAll::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& /*manifest*/)
{
}

sp<BroadPhrase> BroadPhraseAll::BUILDER::build(const Scope& /*args*/)
{
    return sp<BroadPhraseAll>::make();
}

}
