#include "app/impl/collider/collider_impl.h"

#include <algorithm>

#include "core/ark.h"
#include "core/util/log.h"

#include "graphics/base/size.h"
#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_filter.h"
#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/base/rigid_body.h"
#include "app/inf/broad_phrase.h"
#include "app/inf/narrow_phrase.h"
#include "app/traits/shape.h"
#include "app/util/rigid_body_def.h"

namespace ark {

static bool collisionFilterTest(const sp<CollisionFilter>& cf1, const sp<CollisionFilter>& cf2)
{
    return cf1 && cf2 ? cf1->collisionTest(*cf2) : true;
}

ColliderImpl::ColliderImpl(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase, RenderController& renderController)
    : _stub(sp<Stub>::make(std::move(broadPhrases), std::move(narrowPhrase)))
{
    renderController.addPreComposeUpdatable(_stub, sp<BooleanByWeakRef<Stub>>::make(_stub, 1));
}

sp<RigidBody> ColliderImpl::createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotate, sp<Boolean> disposed)
{
    CHECK(type == Collider::BODY_TYPE_KINEMATIC || type == Collider::BODY_TYPE_DYNAMIC || type == Collider::BODY_TYPE_STATIC || type == Collider::BODY_TYPE_SENSOR, "Unknown BodyType: %d", type);
    return _stub->createRigidBody(_stub->generateRigidBodyId(), type, std::move(shape), std::move(position), std::move(rotate), std::move(disposed));
}

std::vector<RayCastManifold> ColliderImpl::rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter)
{
    return _stub->rayCast(V2(from.x(), from.y()), V2(to.x(), to.y()), collisionFilter);
}

ColliderImpl::Stub::Stub(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase)
    : _rigid_body_base_id(0), _broad_phrases(std::move(broadPhrases)), _narrow_phrase(std::move(narrowPhrase))
{
}

BroadPhrase::Result ColliderImpl::Stub::broadPhraseSearch(const V3& position, const V3& aabb, const sp<CollisionFilter>& collisionFilter) const
{
    if(_broad_phrases.size() == 1)
    {
        const auto& [i, j] = _broad_phrases.at(0);
        return collisionFilterTest(j, collisionFilter) ? i->search(position, aabb) : BroadPhrase::Result();
    }

    BroadPhrase::Result result;
    for(const auto& [i, j] : _broad_phrases)
        if(collisionFilterTest(j, collisionFilter))
            result.merge(i->search(position, aabb));

    return result;
}

BroadPhrase::Result ColliderImpl::Stub::broadPhraseRayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) const
{
    if(_broad_phrases.size() == 1)
    {
        const auto& [i, j] = _broad_phrases.at(0);
        return i->rayCast(from, to, collisionFilter);
    }

    BroadPhrase::Result result;
    for(const auto& [i, j] : _broad_phrases)
        result.merge(i->rayCast(from, to, collisionFilter));

    return result;
}

void ColliderImpl::Stub::updateBroadPhraseCandidate(int32_t id, const V3& position, const V3& aabb) const
{
    for(const auto& [i, j] : _broad_phrases)
        i->update(id, position, aabb);
}

void ColliderImpl::Stub::removeBroadPhraseCandidate(int32_t id)
{
    for(const auto& [i, j] : _broad_phrases)
        i->remove(id);
}

bool ColliderImpl::Stub::update(uint64_t timestamp)
{
    DPROFILER_TRACE("CollisionTest", ApplicationProfiler::CATEGORY_PHYSICS);

    _rigid_body_refs.clear();
    _phrase_remove = std::move(_phrase_dispose);

    for(const auto& [id, shadow] : _rigid_bodies)
    {
        const bool isDiscarded = shadow->_discarded.val();
        if(shadow->type() != Collider::BODY_TYPE_STATIC)
        {
            shadow->update(timestamp);
            if(!isDiscarded)
                _rigid_body_refs.push_back(shadow);
        }
        if(isDiscarded)
            _phrase_remove.insert(id);
    }

    for(const sp<RigidBodyShadow>& i : _rigid_body_refs)
        i->collisionTest(i, *this, i->position()->val(), V3(i->bodyDef().occupyRadius() * 2), _phrase_remove);

    for(int32_t i : _phrase_remove)
    {
        const auto iter = _rigid_bodies.find(i);
        DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", i);
        LOGD("Removing RigidBody(%d)", i);
        removeBroadPhraseCandidate(i);
        _rigid_bodies.erase(iter);
    }
    return true;
}

void ColliderImpl::Stub::requestRigidBodyRemoval(int32_t rigidBodyId)
{
    _phrase_dispose.insert(rigidBodyId);
}

int32_t ColliderImpl::Stub::generateRigidBodyId()
{
    return ++_rigid_body_base_id;
}

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(int32_t rigidBodyId, Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotate, sp<Boolean> discarded)
{
    const SafeVar<Vec3> size = shape ? shape->size() : nullptr;
    sp<RigidBodyShadow> rigidBodyShadow = sp<RigidBodyShadow>::make(*this, rigidBodyId, type, 0, std::move(shape), std::move(position), std::move(rotate), std::move(discarded));
    const RigidBodyDef& rigidBodyDef = rigidBodyShadow->updateBodyDef(_narrow_phrase, size);
    _rigid_bodies[rigidBodyShadow->id()] = rigidBodyShadow;

    const float s = rigidBodyDef.occupyRadius() * 2;
    const V3 posVal = position ? position->val() : V3();
    for(const auto& [i, j] : _broad_phrases)
        i->create(rigidBodyId, posVal, V3(s));

    return sp<RigidBodyImpl>::make(std::move(rigidBodyShadow));
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::Stub::ensureRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::ensureRigidBody(int32_t id, uint32_t metaId, int32_t shapeId, const V3& position, bool isDynamicCandidate) const
{
    sp<RigidBodyShadow> s = isDynamicCandidate ? findRigidBody(id) : nullptr;
    DASSERT(!isDynamicCandidate || s);
    return s ? s : sp<RigidBodyShadow>::make(*this, id, isDynamicCandidate ? Collider::BODY_TYPE_DYNAMIC : Collider::BODY_TYPE_STATIC, metaId, sp<Shape>::make(shapeId, sp<Vec3::Const>::make(position)), nullptr, nullptr, nullptr);
}

sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::findRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    return iter != _rigid_bodies.end() ? iter->second : nullptr;
}

std::vector<sp<ColliderImpl::RigidBodyShadow>> ColliderImpl::Stub::toRigidBodyShadows(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const
{
    std::vector<sp<ColliderImpl::RigidBodyShadow>> rigidBodies;
    for(int32_t i : candidateSet)
    {
        sp<RigidBodyShadow> rigidBody = ensureRigidBody(i);
        if(rigidBody->type() & filter)
            rigidBodies.push_back(std::move(rigidBody));
    }
    return rigidBodies;
}

std::vector<BroadPhrase::Candidate> ColliderImpl::Stub::toBroadPhraseCandidates(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const
{
    std::vector<BroadPhrase::Candidate> candidates;
    for(int32_t i : candidateSet)
    {
        const sp<RigidBodyShadow>& rigidBody = ensureRigidBody(i);
        if(rigidBody->type() & filter)
            candidates.emplace_back(rigidBody->toBroadPhraseCandidate());
    }
    return candidates;
}

std::vector<RayCastManifold> ColliderImpl::Stub::rayCast(const V2& from, const V2& to, const sp<CollisionFilter>& collisionFilter) const
{
    std::vector<RayCastManifold> manifolds;
    const BroadPhrase::Result result = broadPhraseRayCast(V3(from.x(), from.y(), 0), V3(to.x(), to.y(), 0), collisionFilter);

    const NarrowPhrase::Ray ray = _narrow_phrase->toRay(from, to);
    for(const auto& i : toRigidBodyShadows(result._dynamic_candidates, Collider::BODY_TYPE_RIGID))
    {
        RayCastManifold raycast(0, V3(0), i);
        if(_narrow_phrase->rayCastManifold(ray, i->toBroadPhraseCandidate(), raycast))
            manifolds.push_back(raycast);
    }
    RayCastManifold raycast;
    for(const auto& i : result._static_candidates)
    {
        if(_narrow_phrase->rayCastManifold(ray, i, raycast))
            manifolds.emplace_back(raycast.distance(), raycast.normal(), sp<RigidBodyShadow>::make(*this, i._id, Collider::BODY_TYPE_STATIC, i._meta_id, sp<Shape>::make(i._shape_id), sp<Vec3::Const>::make(V3(i._position.x(), i._position.y(), 0)), nullptr, nullptr));
    }
    return manifolds;
}

void ColliderImpl::Stub::resolveCandidates(const sp<RigidBody>& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, bool isDynamicCandidates, RigidBody::Callback& callback, std::set<int32_t>& c)
{
    std::set<int32_t> contacts = std::move(c);
    std::set<int32_t> contactsOut;
    for(const BroadPhrase::Candidate& i : candidates)
    {
        CollisionManifold manifold;
        if(_narrow_phrase->collisionManifold(candidateSelf, i, manifold))
        {
            auto iter = contacts.find(i._id);
            if(iter == contacts.end())
                callback.onBeginContact(self, ensureRigidBody(i._id, i._meta_id, i._shape_id, V3(i._position.x(), i._position.y(), 0), isDynamicCandidates), manifold);
            else
                contacts.erase(iter);
            contactsOut.insert(i._id);
        }
    }
    for(int32_t i : contacts)
    {
        if(contactsOut.find(i) == contactsOut.end())
            callback.onEndContact(self, ensureRigidBody(i, 0, 0, V3(), isDynamicCandidates));
    }
    c = std::move(contactsOut);
}

const sp<NarrowPhrase>& ColliderImpl::Stub::narrowPhrase() const
{
    return _narrow_phrase;
}

ColliderImpl::RigidBodyImpl::RigidBodyImpl(sp<RigidBodyShadow> shadow)
    : RigidBody(shadow->stub()), _shadow(std::move(shadow))
{
}

void ColliderImpl::RigidBodyImpl::doDispose()
{
    _shadow->dispose();
}

void ColliderImpl::RigidBodyImpl::dispose()
{
    doDispose();
}

ColliderImpl::RigidBodyShadow::RigidBodyShadow(const ColliderImpl::Stub& stub, uint32_t id, Collider::BodyType type, uint32_t metaId, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotation, SafeVar<Boolean> discarded)
    : RigidBody(sp<RigidBody::Stub>::make(id, type, metaId, std::move(shape), std::move(position), sp<Transform>::make(Transform::TYPE_LINEAR_2D, std::move(rotation)), Box(),
                                          std::move(discarded))), _collider_stub(stub), _position_updated(true), _size_updated(false)
{
}

void ColliderImpl::RigidBodyShadow::dispose()
{
    stub()->_discarded.reset(true);
}

bool ColliderImpl::RigidBodyShadow::update(uint64_t timestamp)
{
    const bool dirty = stub()->_timestamp.update(timestamp);
    if(const SafeVar<Vec3>& size = stub()->_shape->size())
        _size_updated = size.update(timestamp) | _size_updated | dirty;
    _position_updated = position()->update(timestamp) | _position_updated | dirty;

    const V3 pos = position()->val();
    if(_size_updated || _position_updated) {
        if(_size_updated) {
            updateBodyDef(_collider_stub.narrowPhrase(), stub()->_shape->size());
            _size_updated = false;
        }
        float r = bodyDef().occupyRadius();
        _collider_stub.updateBroadPhraseCandidate(id(), pos, V3(r * 2));
        _position_updated = false;
    }
    return true;
}

void ColliderImpl::RigidBodyShadow::collisionTest(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V3& position, const V3& size, const std::set<int32_t>& removingIds)
{
    const Stub& shadowStub = stub();
    if(shadowStub._discarded.val())
        return doDispose(collider);

    BroadPhrase::Result result;
    std::unordered_set<int32_t> dynamicCandidates;

    {
        DPROFILER_TRACE("BroadPhrase");
        result = collider.broadPhraseSearch(position, size, self->collisionFilter());
        dynamicCandidates = std::move(result._dynamic_candidates);
        dynamicCandidates.erase(shadowStub._id);
        for(int32_t i : removingIds)
            dynamicCandidates.erase(i);
    }

    {
        DPROFILER_TRACE("NarrowPhrase");
        const BroadPhrase::Candidate candidateSelf = toBroadPhraseCandidate();
        collider.resolveCandidates(self, candidateSelf, collider.toBroadPhraseCandidates(dynamicCandidates, Collider::BODY_TYPE_ALL), true, shadowStub._callback, _dynamic_contacts);
        collider.resolveCandidates(self, candidateSelf, result._static_candidates, false, shadowStub._callback, _static_contacts);
    }
}

void ColliderImpl::RigidBodyShadow::doDispose(ColliderImpl::Stub& stub)
{
    stub.requestRigidBodyRemoval(id());
}

const RigidBodyDef& ColliderImpl::RigidBodyShadow::bodyDef() const
{
    return _body_def;
}

const RigidBodyDef& ColliderImpl::RigidBodyShadow::updateBodyDef(NarrowPhrase& narrowPhrase, const SafeVar<Vec3>& size)
{
    _body_def = narrowPhrase.makeBodyDef(stub()->_shape->id(), size);
    return _body_def;
}

BroadPhrase::Candidate ColliderImpl::RigidBodyShadow::toBroadPhraseCandidate() const
{
    return BroadPhrase::Candidate(id(), position()->val(), transform()->rotation()->theta()->val(), metaId(), stub()->_shape->id(), collisionFilter(), bodyDef().impl());
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _narrow_phrase(factory.ensureBuilder<NarrowPhrase>(manifest, "narrow-phrase")), _render_controller(resourceLoaderContext->renderController())
{
    for(const document& i : manifest->children("broad-phrase"))
        _broad_phrases.emplace_back(factory.ensureBuilder<BroadPhrase>(i), factory.getBuilder<CollisionFilter>(i, "collision-filter"));

    CHECK(_broad_phrases.size() > 0, "Collider should have at least one BroadPhrase");
}

sp<Collider> ColliderImpl::BUILDER::build(const Scope& args)
{
    std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases;
    for(const auto& [i, j] : _broad_phrases)
        broadPhrases.emplace_back(i->build(args), j->build(args));
    return sp<ColliderImpl>::make(std::move(broadPhrases), _narrow_phrase->build(args), _render_controller);
}

}
