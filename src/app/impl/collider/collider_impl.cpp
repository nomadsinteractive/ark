#include "app/impl/collider/collider_impl.h"

#include <algorithm>
#include <ranges>

#include "core/ark.h"
#include "core/base/ref_manager.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/log.h"

#include "graphics/components/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_filter.h"
#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/components/rigidbody.h"
#include "app/inf/broad_phrase.h"
#include "app/inf/narrow_phrase.h"
#include "graphics/components/shape.h"
#include "app/inf/rigidbody_controller.h"
#include "app/util/rigid_body_def.h"

namespace ark {

namespace {

bool collisionFilterTest(const sp<CollisionFilter>& cf1, const sp<CollisionFilter>& cf2)
{
    return cf1 && cf2 ? cf1->collisionTest(*cf2) : true;
}

}

class ColliderImpl::RigidbodyImpl final : public RigidbodyController {
public:
    RigidbodyImpl(const Stub& stub, Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
        : _rigidbody_stub(sp<Rigidbody::Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter))), _collider_stub(stub), _position_updated(true), _size_updated(false)
    {
    }

    bool update(const uint64_t timestamp)
    {
        if(const OptionalVar<Vec3>& size = _rigidbody_stub->_shape->size())
            _size_updated = size.update(timestamp) | _size_updated;
        _position_updated = _rigidbody_stub->_position.update(timestamp) | _position_updated;

        const V3 pos = _rigidbody_stub->_position.val();
        if(_size_updated || _position_updated)
        {
            if(_size_updated)
            {
                updateBodyDef(_collider_stub.narrowPhrase(), _rigidbody_stub->_shape->size().toVar());
                _size_updated = false;
            }
            const float r = bodyDef().occupyRadius();
            _collider_stub.updateBroadPhraseCandidate(_rigidbody_stub->_ref->id(), pos, V3(r * 2));
            _position_updated = false;
            return true;
        }
        return false;
    }

    void collisionTest(Stub& collider, const V3& position, const V3& size, const Set<BroadPhrase::CandidateIdType>& removingIds)
    {
        if(_rigidbody_stub->_ref->isDiscarded())
            return doDiscard(collider);

        BroadPhrase::Result result;
        HashSet<BroadPhrase::CandidateIdType> dynamicCandidates;

        {
            DPROFILER_TRACE("BroadPhrase");
            result = collider.broadPhraseSearch(position, size, _rigidbody_stub->_collision_filter);
            dynamicCandidates = std::move(result._dynamic_candidates);
            dynamicCandidates.erase(_rigidbody_stub->_ref->id());
            for(const BroadPhrase::CandidateIdType i : removingIds)
                dynamicCandidates.erase(i);
        }

        {
            DPROFILER_TRACE("NarrowPhrase");
            const BroadPhrase::Candidate candidateSelf = toBroadPhraseCandidate();
            const Rigidbody self({_rigidbody_stub, nullptr}, true);
            collider.resolveCandidates(self, candidateSelf, collider.toBroadPhraseCandidates(dynamicCandidates), _dynamic_contacts);
            collider.resolveCandidates(self, candidateSelf, result._static_candidates, _static_contacts);
        }
    }

    void doDiscard(Stub& stub) const
    {
        stub.requestRigidBodyRemoval(_rigidbody_stub->_ref->id());
    }

    const RigidbodyDef& bodyDef() const
    {
        return _body_def;
    }

    const RigidbodyDef& updateBodyDef(NarrowPhrase& narrowPhrase, sp<Vec3> size)
    {
        _body_def = narrowPhrase.makeBodyDef(_rigidbody_stub->_shape->type().hash(), std::move(size));
        return _body_def;
    }

    BroadPhrase::Candidate toBroadPhraseCandidate() const
    {
        return {_rigidbody_stub->_ref->id(), _rigidbody_stub->_position.val(), _rigidbody_stub->_rotation.val(), _rigidbody_stub->_shape, _rigidbody_stub->_collision_filter};
    }

    Rigidbody makeShadow() const
    {
        return {Rigidbody::Impl{_rigidbody_stub, nullptr}, true};
    }

    V3 linearVelocity() const override
    {
        FATAL("Unimplemented");
        return V3(0);
    }

    void setLinearVelocity(const V3& velocity) override
    {
        FATAL("Unimplemented");
    }

    void applyCentralImpulse(const V3& impulse) override
    {
        FATAL("Unimplemented");
    }

    sp<Rigidbody::Stub> _rigidbody_stub;
    const Stub& _collider_stub;
    Set<BroadPhrase::CandidateIdType> _dynamic_contacts;
    Set<BroadPhrase::CandidateIdType> _static_contacts;

    RigidbodyDef _body_def;

    bool _position_updated;
    bool _size_updated;
};

ColliderImpl::ColliderImpl(Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase, RenderController& renderController)
    : _stub(sp<Stub>::make(std::move(broadPhrases), std::move(narrowPhrase)))
{
    renderController.addPreComposeUpdatable(_stub, sp<Boolean>::make<BooleanByWeakRef<Stub>>(_stub, 1));
}

Rigidbody::Impl ColliderImpl::createBody(const Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    CHECK(type == Rigidbody::BODY_TYPE_KINEMATIC || type == Rigidbody::BODY_TYPE_DYNAMIC || type == Rigidbody::BODY_TYPE_STATIC || type == Rigidbody::BODY_TYPE_SENSOR, "Unknown BodyType: %d", type);
    if(!shape->implementation())
        shape = createShape(shape->type(), shape->size() ? shape->size().toVar() : sp<Vec3>(), shape->origin() ? shape->origin().toVar() : sp<Vec3>());
    const sp<RigidbodyImpl> rigidbodyImpl = _stub->createRigidBody(type, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter), std::move(discarded));
    sp<Rigidbody::Stub> stub = rigidbodyImpl->_rigidbody_stub;
    return Rigidbody::Impl{std::move(stub), nullptr, rigidbodyImpl};
}

sp<Shape> ColliderImpl::createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
{
    auto [_implementation, _size] = _stub->narrowPhrase()->createShapeDef(type.hash(), size ? Optional<V3>(size->val()) : Optional<V3>());
    return sp<Shape>::make(type, size ? std::move(size) : sp<Vec3>::make<Vec3::Const>(_size), std::move(origin), std::move(_implementation));
}

Vector<RayCastManifold> ColliderImpl::rayCast(V3 from, V3 to, const sp<CollisionFilter>& collisionFilter)
{
    return _stub->rayCast(V2(from.x(), from.y()), V2(to.x(), to.y()), collisionFilter);
}

ColliderImpl::Stub::Stub(Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase)
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
    for(const auto& i : _broad_phrases | std::views::keys)
        result.merge(i->rayCast(from, to, collisionFilter));

    return result;
}

void ColliderImpl::Stub::updateBroadPhraseCandidate(BroadPhrase::CandidateIdType id, const V3& position, const V3& aabb) const
{
    for(const auto& i : _broad_phrases | std::views::keys)
        i->update(id, position, aabb);
}

void ColliderImpl::Stub::removeBroadPhraseCandidate(BroadPhrase::CandidateIdType id)
{
    for(const auto& i : _broad_phrases | std::views::keys)
        i->remove(id);
}

bool ColliderImpl::Stub::update(const uint64_t timestamp)
{
    DPROFILER_TRACE("CollisionTest", ApplicationProfiler::CATEGORY_PHYSICS);

    _dirty_rigid_body_refs.clear();
    _phrase_remove = std::move(_phrase_dispose);

    for(const auto& [id, ref] : _rigid_bodies)
        if(ref && !ref->isDiscarded())
        {
            RigidbodyImpl& body = ref->instance<RigidbodyImpl>();
            const bool dirty = body.update(timestamp);
            if(body._rigidbody_stub->_ref->discarded().val())
                _phrase_remove.insert(id);
            else if(dirty && body._rigidbody_stub->_type != Rigidbody::BODY_TYPE_STATIC)
                _dirty_rigid_body_refs.insert(ref.get());
        }
        else
            _phrase_remove.insert(id);

    for(const Ref* i : _dirty_rigid_body_refs)
    {
        auto& bodyImpl = i->instance<RigidbodyImpl>();
        bodyImpl.collisionTest(*this, bodyImpl._rigidbody_stub->_position.val(), V3(bodyImpl.bodyDef().occupyRadius() * 2), _phrase_remove);
    }

    for(const BroadPhrase::CandidateIdType i : _phrase_remove)
    {
        const auto iter = _rigid_bodies.find(i);
        DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", i);
        LOGD("Removing RigidBody(%d)", i);
        removeBroadPhraseCandidate(i);
        _rigid_bodies.erase(iter);
    }
    return true;
}

void ColliderImpl::Stub::requestRigidBodyRemoval(const int32_t rigidBodyId)
{
    _phrase_dispose.insert(rigidBodyId);
}

sp<ColliderImpl::RigidbodyImpl> ColliderImpl::Stub::createRigidBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotate, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    const V3 size = shape->size().val();
    sp<RigidbodyImpl> rigidBody = sp<RigidbodyImpl>::make(*this, type, std::move(shape), std::move(position), std::move(rotate), std::move(collisionFilter), std::move(discarded));
    const RigidbodyDef& rigidBodyDef = rigidBody->updateBodyDef(_narrow_phrase, sp<Vec3>::make<Vec3::Const>(size));
    _rigid_bodies[rigidBody->_rigidbody_stub->_ref->id()] = rigidBody->_rigidbody_stub->_ref;

    const V3 posVal = rigidBody->_rigidbody_stub->_position.val();
    const float s = rigidBodyDef.occupyRadius() * 2;
    for(const auto& i : _broad_phrases | std::views::keys)
        i->create(rigidBody->_rigidbody_stub->_ref->id(), posVal, V3(s));

    return rigidBody;
}

Vector<sp<Ref>> ColliderImpl::Stub::toRigidBodyRefs(const HashSet<BroadPhrase::CandidateIdType>& candidateSet, uint32_t filter) const
{
    Vector<sp<Ref>> rigidBodies;
    for(BroadPhrase::CandidateIdType i : candidateSet)
        if(const auto iter = _rigid_bodies.find(i); iter != _rigid_bodies.end())
            if(const sp<Ref>& ref = iter->second; !ref->isDiscarded() && ref->instance<RigidbodyImpl>()._rigidbody_stub->_type & filter)
                rigidBodies.push_back(ref);
    return rigidBodies;
}

Vector<BroadPhrase::Candidate> ColliderImpl::Stub::toBroadPhraseCandidates(const HashSet<BroadPhrase::CandidateIdType>& candidateSet)
{
    Vector<BroadPhrase::Candidate> candidates;
    const RefManager& refManager = Global<RefManager>();
    for(const BroadPhrase::CandidateIdType i : candidateSet)
    {
        const RigidbodyImpl& rigidBody = refManager.toRef(i)->instance<RigidbodyImpl>();
        candidates.emplace_back(rigidBody.toBroadPhraseCandidate());
    }
    return candidates;
}

Vector<RayCastManifold> ColliderImpl::Stub::rayCast(const V2& from, const V2& to, const sp<CollisionFilter>& collisionFilter) const
{
    Vector<RayCastManifold> manifolds;
    const BroadPhrase::Result result = broadPhraseRayCast(V3(from.x(), from.y(), 0), V3(to.x(), to.y(), 0), collisionFilter);

    const NarrowPhrase::Ray ray = _narrow_phrase->toRay(from, to);
    for(const auto& i : toRigidBodyRefs(result._dynamic_candidates, Rigidbody::BODY_TYPE_RIGID))
    {
        const RigidbodyImpl& impl = i->instance<RigidbodyImpl>();
        if(Optional<RayCastManifold> raycast = _narrow_phrase->rayCastManifold(ray, impl.toBroadPhraseCandidate()))
        {
            raycast->setRigidBody(sp<Rigidbody>::make(impl.makeShadow()));
            manifolds.push_back(std::move(raycast.value()));
        }
    }
    for(const auto& i : result._static_candidates)
        if(Optional<RayCastManifold> raycast = _narrow_phrase->rayCastManifold(ray, i))
            if(const auto iter = _rigid_bodies.find(i._id); iter != _rigid_bodies.end())
            {
                const RigidbodyImpl& impl = iter->second->instance<RigidbodyImpl>();
                manifolds.emplace_back(raycast->distance(), raycast->normal(), sp<Rigidbody>::make(impl.makeShadow()));
            }
    return manifolds;
}

void ColliderImpl::Stub::resolveCandidates(const Rigidbody& self, const BroadPhrase::Candidate& candidateSelf, const Vector<BroadPhrase::Candidate>& candidates, Set<BroadPhrase::CandidateIdType>& c) const
{
    Set<BroadPhrase::CandidateIdType> contacts = std::move(c);
    Set<BroadPhrase::CandidateIdType> contactsOut;
    const RefManager& refManager = Global<RefManager>();
    for(const BroadPhrase::Candidate& i : candidates)
    {
        CollisionManifold manifold;
        if(_narrow_phrase->collisionManifold(candidateSelf, i, manifold))
        {
            if(const auto iter = contacts.find(i._id); iter == contacts.end())
            {
                if(const Ref& ref = refManager.toRef(i._id))
                {
                    const Rigidbody other = ref.instance<RigidbodyImpl>().makeShadow();
                    self.onBeginContact(other, manifold);
                    if(!_dirty_rigid_body_refs.contains(&ref))
                        other.onBeginContact(self, {manifold.contactPoint(), -manifold.normal()});
                }
            }
            else
                contacts.erase(iter);
            contactsOut.insert(i._id);
        }
    }
    for(const BroadPhrase::CandidateIdType i : contacts)
    {
        if(!contactsOut.contains(i))
            if(const Ref& ref = refManager.toRef(i))
            {
                const Rigidbody other = ref.instance<RigidbodyImpl>().makeShadow();
                self.onEndContact(other);
                if(!_dirty_rigid_body_refs.contains(&ref))
                    other.onEndContact(self);
            }
    }
    c = std::move(contactsOut);
}

const sp<NarrowPhrase>& ColliderImpl::Stub::narrowPhrase() const
{
    return _narrow_phrase;
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _narrow_phrase(factory.ensureBuilder<NarrowPhrase>(manifest, "narrow-phrase")), _render_controller(resourceLoaderContext->renderController())
{
    for(const document& i : manifest->children("broad-phrase"))
        _broad_phrases.emplace_back(factory.ensureBuilder<BroadPhrase>(i), factory.getBuilder<CollisionFilter>(i, "collision-filter"));

    CHECK(!_broad_phrases.empty(), "Collider should have at least one BroadPhrase");
}

sp<Collider> ColliderImpl::BUILDER::build(const Scope& args)
{
    Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases;
    for(const auto& [i, j] : _broad_phrases)
        broadPhrases.emplace_back(i->build(args), j.build(args));
    return sp<Collider>::make<ColliderImpl>(std::move(broadPhrases), _narrow_phrase->build(args), _render_controller);
}

}
