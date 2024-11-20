#include "app/impl/collider/collider_impl.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/ref_manager.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/log.h"

#include "graphics/base/size.h"
#include "graphics/inf/transform.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_filter.h"
#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/base/rigidbody.h"
#include "app/inf/broad_phrase.h"
#include "app/inf/narrow_phrase.h"
#include "app/traits/shape.h"
#include "app/util/rigid_body_def.h"

namespace ark {

namespace {

bool collisionFilterTest(const sp<CollisionFilter>& cf1, const sp<CollisionFilter>& cf2)
{
    return cf1 && cf2 ? cf1->collisionTest(*cf2) : true;
}

}

ColliderImpl::ColliderImpl(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase, RenderController& renderController)
    : _stub(sp<Stub>::make(std::move(broadPhrases), std::move(narrowPhrase)))
{
    renderController.addPreComposeUpdatable(_stub, sp<BooleanByWeakRef<Stub>>::make(_stub, 1));
}

sp<Rigidbody> ColliderImpl::createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded)
{
    CHECK(type == Collider::BODY_TYPE_KINEMATIC || type == Collider::BODY_TYPE_DYNAMIC || type == Collider::BODY_TYPE_STATIC || type == Collider::BODY_TYPE_SENSOR, "Unknown BodyType: %d", type);
    return _stub->createRigidBody(type, std::move(shape), std::move(position), std::move(rotation), std::move(discarded));
}

sp<Shape> ColliderImpl::createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
{
    return sp<Shape>::make(type, std::move(size), std::move(origin));
}

std::vector<RayCastManifold> ColliderImpl::rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter)
{
    return _stub->rayCast(V2(from.x(), from.y()), V2(to.x(), to.y()), collisionFilter);
}

ColliderImpl::Stub::Stub(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase)
    : _broad_phrases(std::move(broadPhrases)), _narrow_phrase(std::move(narrowPhrase))
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

void ColliderImpl::Stub::updateBroadPhraseCandidate(BroadPhrase::IdType id, const V3& position, const V3& aabb) const
{
    for(const auto& [i, j] : _broad_phrases)
        i->update(id, position, aabb);
}

void ColliderImpl::Stub::removeBroadPhraseCandidate(BroadPhrase::IdType id)
{
    for(const auto& [i, j] : _broad_phrases)
        i->remove(id);
}

bool ColliderImpl::Stub::update(uint64_t timestamp)
{
    DPROFILER_TRACE("CollisionTest", ApplicationProfiler::CATEGORY_PHYSICS);

    _rigid_body_refs.clear();
    _phrase_remove = std::move(_phrase_dispose);

    for(const auto& [id, ref] : _rigid_bodies)
    {
        if(ref)
        {
            RigidBodyImpl& body = ref->instance<RigidBodyImpl>();
            body.update(timestamp);
            if(body.discarded().val())
                _phrase_remove.insert(id);
            else if(body.type() != Collider::BODY_TYPE_STATIC)
                _rigid_body_refs.push_back(ref);
        }
        else
            _phrase_remove.insert(id);
    }

    for(Ref& i : _rigid_body_refs)
        i.instance<RigidBodyImpl>().collisionTest(*this, i.instance<RigidBodyImpl>().position().val(), V3(i.instance<RigidBodyImpl>().bodyDef().occupyRadius() * 2), _phrase_remove);

    for(const int32_t i : _phrase_remove)
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

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotate, sp<Boolean> discarded)
{
    const V3 size = shape->size().val();
    sp<RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(*this, type, std::move(shape), std::move(position), std::move(rotate), std::move(discarded));
    const RigidbodyDef& rigidBodyDef = rigidBody->updateBodyDef(_narrow_phrase, sp<Vec3>::make<Vec3::Const>(size));
    _rigid_bodies[rigidBody->ref()->id()] = rigidBody->ref();

    const V3 posVal = rigidBody->position().val();
    float s = rigidBodyDef.occupyRadius() * 2;
    for(const auto& [i, j] : _broad_phrases)
        i->create(rigidBody->ref()->id(), posVal, V3(s));

    return rigidBody;
}

std::vector<sp<Ref>> ColliderImpl::Stub::toRigidBodyRefs(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const
{
    std::vector<sp<Ref>> rigidBodies;
    for(BroadPhrase::IdType i : candidateSet)
        if(const auto iter = _rigid_bodies.find(i); iter != _rigid_bodies.end())
            if(const sp<Ref>& ref = iter->second; !ref->isDiscarded() && ref->instance<Rigidbody>().type() & filter)
                rigidBodies.push_back(ref);
    return rigidBodies;
}

std::vector<BroadPhrase::Candidate> ColliderImpl::Stub::toBroadPhraseCandidates(const std::unordered_set<BroadPhrase::IdType>& candidateSet) const
{
    std::vector<BroadPhrase::Candidate> candidates;
    RefManager& refManager = Global<RefManager>();
    for(BroadPhrase::IdType i : candidateSet)
    {
        const RigidBodyImpl& rigidBody = refManager.toRef(i).instance<RigidBodyImpl>();
        candidates.emplace_back(rigidBody.toBroadPhraseCandidate());
    }
    return candidates;
}

std::vector<RayCastManifold> ColliderImpl::Stub::rayCast(const V2& from, const V2& to, const sp<CollisionFilter>& collisionFilter) const
{
    std::vector<RayCastManifold> manifolds;
    const BroadPhrase::Result result = broadPhraseRayCast(V3(from.x(), from.y(), 0), V3(to.x(), to.y(), 0), collisionFilter);

    const NarrowPhrase::Ray ray = _narrow_phrase->toRay(from, to);
    for(const auto& i : toRigidBodyRefs(result._dynamic_candidates, Collider::BODY_TYPE_RIGID))
    {
        RayCastManifold raycast(0, V3(0), i);
        if(_narrow_phrase->rayCastManifold(ray, i->instance<RigidBodyImpl>().toBroadPhraseCandidate(), raycast))
            manifolds.push_back(raycast);
    }
    RayCastManifold raycast;
    for(const auto& i : result._static_candidates)
    {
        if(_narrow_phrase->rayCastManifold(ray, i, raycast))
            if(const auto iter = _rigid_bodies.find(i._id); iter != _rigid_bodies.end())
                manifolds.emplace_back(raycast.distance(), raycast.normal(), iter->second);
    }
    return manifolds;
}

void ColliderImpl::Stub::resolveCandidates(const Rigidbody& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, const Rigidbody& callback, std::set<BroadPhrase::IdType>& c)
{
    std::set<BroadPhrase::IdType> contacts = std::move(c);
    std::set<BroadPhrase::IdType> contactsOut;
    RefManager& refManager = Global<RefManager>();
    for(const BroadPhrase::Candidate& i : candidates)
    {
        CollisionManifold manifold;
        if(_narrow_phrase->collisionManifold(candidateSelf, i, manifold))
        {
            if(const auto iter = contacts.find(i._id); iter == contacts.end())
            {
                if(const Ref& ref = refManager.toRef(i._id))
                    callback.onBeginContact(self, ref.instance<Rigidbody>(), manifold);
            }
            else
                contacts.erase(iter);
            contactsOut.insert(i._id);
        }
    }
    for(const BroadPhrase::IdType i : contacts)
    {
        if(contactsOut.find(i) == contactsOut.end())
            if(const Ref& ref = refManager.toRef(i))
                callback.onEndContact(self, ref.instance<Rigidbody>());
    }
    c = std::move(contactsOut);
}

const sp<NarrowPhrase>& ColliderImpl::Stub::narrowPhrase() const
{
    return _narrow_phrase;
}

ColliderImpl::RigidBodyImpl::RigidBodyImpl(const ColliderImpl::Stub& stub, Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded)
    : Rigidbody(type, std::move(shape), std::move(position), std::move(rotation), Box(), Global<RefManager>()->makeRef(this, std::move(discarded))), _collider_stub(stub), _position_updated(true), _size_updated(false)
{
}

bool ColliderImpl::RigidBodyImpl::update(uint64_t timestamp)
{
    if(const SafeVar<Vec3>& size = _shape->size())
        _size_updated = size.update(timestamp) | _size_updated;
    _position_updated = position().update(timestamp) | _position_updated;

    const V3 pos = position().val();
    if(_size_updated || _position_updated) {
        if(_size_updated) {
            updateBodyDef(_collider_stub.narrowPhrase(), _shape->size());
            _size_updated = false;
        }
        const float r = bodyDef().occupyRadius();
        _collider_stub.updateBroadPhraseCandidate(ref()->id(), pos, V3(r * 2));
        _position_updated = false;
    }
    return true;
}

void ColliderImpl::RigidBodyImpl::collisionTest(ColliderImpl::Stub& collider, const V3& position, const V3& size, const std::set<BroadPhrase::IdType>& removingIds)
{
    if(_ref->isDiscarded())
        return doDispose(collider);

    BroadPhrase::Result result;
    std::unordered_set<BroadPhrase::IdType> dynamicCandidates;

    {
        DPROFILER_TRACE("BroadPhrase");
        result = collider.broadPhraseSearch(position, size, collisionFilter());
        dynamicCandidates = std::move(result._dynamic_candidates);
        dynamicCandidates.erase(ref()->id());
        for(const BroadPhrase::IdType i : removingIds)
            dynamicCandidates.erase(i);
    }

    {
        DPROFILER_TRACE("NarrowPhrase");
        const BroadPhrase::Candidate candidateSelf = toBroadPhraseCandidate();
        collider.resolveCandidates(*this, candidateSelf, collider.toBroadPhraseCandidates(dynamicCandidates), *this, _dynamic_contacts);
        collider.resolveCandidates(*this, candidateSelf, result._static_candidates, *this, _static_contacts);
    }
}

void ColliderImpl::RigidBodyImpl::doDispose(ColliderImpl::Stub& stub)
{
    stub.requestRigidBodyRemoval(ref()->id());
}

const RigidbodyDef& ColliderImpl::RigidBodyImpl::bodyDef() const
{
    return _body_def;
}

const RigidbodyDef& ColliderImpl::RigidBodyImpl::updateBodyDef(NarrowPhrase& narrowPhrase, const SafeVar<Vec3>& size)
{
    _body_def = narrowPhrase.makeBodyDef(_shape->type().hash(), size);
    return _body_def;
}

BroadPhrase::Candidate ColliderImpl::RigidBodyImpl::toBroadPhraseCandidate() const
{
    return {ref()->id(), position().val(), quaternion().val(), metaId(), _shape->type().hash(), collisionFilter(), bodyDef().impl()};
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
        broadPhrases.emplace_back(i->build(args), j.build(args));
    return sp<ColliderImpl>::make(std::move(broadPhrases), _narrow_phrase->build(args), _render_controller);
}

}
