#include "app/impl/collider/collider_impl.h"

#include <algorithm>
#include <ranges>

#include "core/ark.h"
#include "core/base/ref_manager.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/log.h"

#include "graphics/components/shape.h"
#include "graphics/components/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_filter.h"
#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/components/rigidbody.h"
#include "app/inf/broad_phrase.h"
#include "app/inf/broad_phrase_callback.h"
#include "app/inf/narrow_phrase.h"
#include "app/inf/rigidbody_controller.h"

namespace ark {

namespace {

bool collisionFilterTest(const sp<CollisionFilter>& cf1, const sp<CollisionFilter>& cf2)
{
    return cf1 && cf2 ? cf1->collisionTest(*cf2) : true;
}

float calcOccupyRadius(const Shape& shape)
{
    const V3 size = shape.scale().value();
    const V3 pivot = shape.origin();
    const V3 pm(std::max(std::abs(pivot.x()), std::abs(1.0f - pivot.x())), std::max(std::abs(pivot.y()), std::abs(1.0f - pivot.y())), std::max(std::abs(pivot.z()), std::abs(1.0f - pivot.z())));
    const V3 sm = pm * size;
    return Math::sqrt(Math::hypot2(sm));
}

}

class ColliderImpl::RigidbodyImpl final : public RigidbodyController {
public:
    RigidbodyImpl(const Stub& stub, Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
        : _rigidbody_stub(sp<Rigidbody::Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter))), _collider_stub(stub), _position_updated(true),
          _occupy_radius(0)
    {
    }

    bool update(const uint64_t timestamp)
    {
        _position_updated = _rigidbody_stub->_position.update(timestamp) | _position_updated;

        const V3 pos = _rigidbody_stub->_position.val();
        if(_position_updated)
        {
            _collider_stub.updateBroadPhraseCandidate(_rigidbody_stub->_ref->id(), pos, V3(_occupy_radius * 2));
            _position_updated = false;
            return true;
        }
        return false;
    }

    void collisionTest(Stub& collider, const V3& position, const V3& size, const Set<RefId>& removingIds)
    {
        if(_rigidbody_stub->_ref->isDiscarded())
            return doDiscard(collider);

        BroadPhraseCallbackCollisionTest broadPhraseCallback(collider, *this, removingIds);
        collider.broadPhraseSearch(broadPhraseCallback, position, size, _rigidbody_stub->_collision_filter);
    }

    void doDiscard(Stub& stub) const
    {
        stub.requestRigidBodyRemoval(_rigidbody_stub->_ref->id());
    }

    BroadPhrase::Candidate toBroadPhraseCandidate() const
    {
        return {_rigidbody_stub->_ref->id(), _rigidbody_stub->_position.val(), _rigidbody_stub->_rotation.val(), _rigidbody_stub->_shape, _rigidbody_stub->_collision_filter};
    }

    Rigidbody makeShadow() const
    {
        return {Rigidbody::Impl{_rigidbody_stub, nullptr}, true};
    }

    bool active() const override
    {
        return true;
    }

    void setActive(bool active) override
    {
    }

    V3 linearVelocity() const override
    {
        FATAL("Unimplemented");
        return V3(0);
    }

    void setLinearVelocity(const V3 velocity) override
    {
        FATAL("Unimplemented");
    }

    V3 angularVelocity() const override
    {
        FATAL("Unimplemented");
        return V3(0);
    }

    void setAngularVelocity(V3 velocity) override
    {
        FATAL("Unimplemented");
    }

    V3 angularFactor() const override
    {
        FATAL("Unimplemented");
        return V3(0);
    }

    void setAngularFactor(V3 factor) override
    {
        FATAL("Unimplemented");
    }

    void applyCentralImpulse(const V3 impulse) override
    {
        FATAL("Unimplemented");
    }

    float friction() const override
    {
        return 0;
    }

    void setFriction(float friction) override
    {
    }

    float updateOccupyRadius()
    {
        _occupy_radius = calcOccupyRadius(_rigidbody_stub->_shape);
        return _occupy_radius;
    }

    sp<Rigidbody::Stub> _rigidbody_stub;
    const Stub& _collider_stub;
    Set<RefId> _dynamic_contacts;
    Set<RefId> _static_contacts;

    bool _position_updated;

    float _occupy_radius;

    class BroadPhraseCallbackCollisionTest final : public BroadPhraseCallback {
    public:
        BroadPhraseCallbackCollisionTest(ColliderImpl::Stub& collider, RigidbodyImpl& rigidbody, const Set<RefId>& removingIds)
            : _collider(collider), _rigidbody(rigidbody), _removing_ids(removingIds), _self(rigidbody.makeShadow()), _candidate_self(rigidbody.toBroadPhraseCandidate()), _narrow_phrase(_collider.narrowPhrase()),
              _dynamic_contacts(std::move(rigidbody._dynamic_contacts)), _static_contacts(std::move(rigidbody._static_contacts))
        {
        }
        ~BroadPhraseCallbackCollisionTest() override
        {
            collisionTestDone(_dynamic_contacts, _dynamic_contacts_out);
            collisionTestDone(_static_contacts, _static_contacts_out);
            _rigidbody._dynamic_contacts = std::move(_dynamic_contacts_out);
            _rigidbody._static_contacts = std::move(_static_contacts_out);
        }

        void onRigidbodyCandidate(const RefId rigidbodyId) override
        {
            if(rigidbodyId != _self.id() && !_removing_ids.contains(rigidbodyId))
            {
                const RefManager& refManager = Global<RefManager>();
                const RigidbodyImpl& rigidBody = refManager.toRef(rigidbodyId)->instance<RigidbodyImpl>();
                collisionTestOne(rigidBody.toBroadPhraseCandidate(), _dynamic_contacts, _dynamic_contacts_out);
            }
        }

        void onStaticCandidate(const RefId candidateId, const V3 position, const V4 quaternion, sp<Shape> shape, sp<CollisionFilter> collisionFilter) override
        {
            collisionTestOne({candidateId, position, quaternion, std::move(shape), std::move(collisionFilter)}, _static_contacts, _static_contacts_out);
        }

        void collisionTestOne(const BroadPhrase::Candidate& candidate, Set<RefId>& contacts, Set<RefId>& contactsOut) const
        {
            const RefManager& refManager = Global<RefManager>();
            CollisionManifold manifold;
            if(_narrow_phrase->collisionManifold(_candidate_self, candidate, manifold))
            {
                if(const auto iter = contacts.find(candidate._id); iter == contacts.end())
                {
                    if(const Ref& ref = refManager.toRef(candidate._id))
                    {
                        const Rigidbody other = ref.instance<RigidbodyImpl>().makeShadow();
                        _self.onBeginContact(other, manifold);
                        if(!_collider._dirty_rigid_body_refs.contains(&ref))
                            other.onBeginContact(_self, {manifold.contactPoint(), -manifold.normal()});
                    }
                }
                else
                    contacts.erase(iter);
                contactsOut.insert(candidate._id);
            }
        }

        void collisionTestDone(const Set<RefId>& contacts, const Set<RefId>& contactsOut) const
        {
            const RefManager& refManager = Global<RefManager>();
            for(const RefId i : contacts)
            {
                if(!contactsOut.contains(i))
                    if(const Ref& ref = refManager.toRef(i))
                    {
                        const Rigidbody other = ref.instance<RigidbodyImpl>().makeShadow();
                        _self.onEndContact(other);
                        if(!_collider._dirty_rigid_body_refs.contains(&ref))
                            other.onEndContact(_self);
                    }
            }
        }

        ColliderImpl::Stub& _collider;
        RigidbodyImpl& _rigidbody;
        const Set<RefId>& _removing_ids;

        Rigidbody _self;
        BroadPhrase::Candidate _candidate_self;
        sp<NarrowPhrase> _narrow_phrase;

        Set<RefId> _dynamic_contacts;
        Set<RefId> _dynamic_contacts_out;

        Set<RefId> _static_contacts;
        Set<RefId> _static_contacts_out;
    };

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
        shape = createShape(shape->type(), shape->scale(), shape->origin());
    const sp<RigidbodyImpl> rigidbodyImpl = _stub->createRigidBody(type, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter), std::move(discarded));
    sp<Rigidbody::Stub> stub = rigidbodyImpl->_rigidbody_stub;
    return Rigidbody::Impl{std::move(stub), nullptr, rigidbodyImpl};
}

sp<Shape> ColliderImpl::createShape(const NamedHash& type, Optional<V3> scale, const V3 origin)
{
    auto [_implementation, _size] = _stub->narrowPhrase()->createShapeDef(type.hash(), scale);
    return sp<Shape>::make(type, scale ? std::move(scale) : Optional<V3>(_size), origin, std::move(_implementation));
}

Vector<RayCastManifold> ColliderImpl::rayCast(const V3 from, const V3 to, const sp<CollisionFilter>& collisionFilter)
{
    return _stub->rayCast(from, to, collisionFilter);
}

ColliderImpl::Stub::Stub(Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase)
    : _broad_phrases(std::move(broadPhrases)), _narrow_phrase(std::move(narrowPhrase))
{
}

void ColliderImpl::Stub::broadPhraseSearch(BroadPhraseCallback& callback, const V3 position, const V3 size, const sp<CollisionFilter>& collisionFilter) const
{
    for(const auto& [i, j] : _broad_phrases)
        if(collisionFilterTest(j, collisionFilter))
            i->search(callback, position, size);
}

void ColliderImpl::Stub::updateBroadPhraseCandidate(const RefId id, const V3& position, const V3& size) const
{
    for(const auto& i : _broad_phrases | std::views::keys)
        if(const auto coordinator = i->requestCoordinator())
            coordinator->update(id, position, size);
}

void ColliderImpl::Stub::removeBroadPhraseCandidate(const RefId id)
{
    for(const auto& i : _broad_phrases | std::views::keys)
        if(const auto coordinator = i->requestCoordinator())
            coordinator->remove(id);
}

bool ColliderImpl::Stub::update(const uint64_t timestamp)
{
    DPROFILER_TRACE("CollisionTest", ApplicationProfiler::CATEGORY_PHYSICS);

    _dirty_rigid_body_refs.clear();
    _phrase_remove = std::move(_phrase_discard);

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
        const float r = bodyImpl._occupy_radius;
        bodyImpl.collisionTest(*this, bodyImpl._rigidbody_stub->_position.val(), V3(r * 2), _phrase_remove);
    }

    for(const RefId i : _phrase_remove)
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
    _phrase_discard.insert(rigidBodyId);
}

sp<ColliderImpl::RigidbodyImpl> ColliderImpl::Stub::createRigidBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotate, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    sp<RigidbodyImpl> rigidBody = sp<RigidbodyImpl>::make(*this, type, std::move(shape), std::move(position), std::move(rotate), std::move(collisionFilter), std::move(discarded));
    _rigid_bodies[rigidBody->_rigidbody_stub->_ref->id()] = rigidBody->_rigidbody_stub->_ref;

    const V3 posVal = rigidBody->_rigidbody_stub->_position.val();
    const float s = rigidBody->updateOccupyRadius() * 2;
    for(const auto& i : _broad_phrases | std::views::keys)
        if(const auto coordinator = i->requestCoordinator())
            coordinator->create(rigidBody->_rigidbody_stub->_ref->id(), posVal, V3(s));

    return rigidBody;
}

sp<Ref> ColliderImpl::Stub::toRigidbodyRef(const RefId refid) const
{
    if(const auto iter = _rigid_bodies.find(refid); iter != _rigid_bodies.end())
        if(const sp<Ref>& ref = iter->second; !ref->isDiscarded())
            return ref;
    return nullptr;
}

class BroadPhraseCallbackRayCast final : public BroadPhraseCallback {
public:
    BroadPhraseCallbackRayCast(const ColliderImpl::Stub& collider, const V3 from, const V3 to)
        : _collider(collider), _narrow_phrase(_collider.narrowPhrase()), _ray(_narrow_phrase->toRay(from, to)) {
    }

    void onRigidbodyCandidate(const RefId rigidbodyId) override
    {
        if(const sp<Ref> ref = _collider.toRigidbodyRef(rigidbodyId))
            if(const ColliderImpl::RigidbodyImpl& rigidbody = ref->instance<ColliderImpl::RigidbodyImpl>(); rigidbody._rigidbody_stub->_type & Rigidbody::BODY_TYPE_RIGID)
                if(Optional<RayCastManifold> raycast = _narrow_phrase->rayCastManifold(_ray, rigidbody.toBroadPhraseCandidate()))
                {
                    raycast->setRigidBody(sp<Rigidbody>::make(rigidbody.makeShadow()));
                    _manifolds.push_back(std::move(raycast.value()));
                }
    }

    void onStaticCandidate(RefId candidateId, V3 position, V4 quaternion, sp<Shape> shape, sp<CollisionFilter> collisionFilter) override
    {
        if(const sp<Ref> ref = _collider.toRigidbodyRef(candidateId))
            if(Optional<RayCastManifold> raycast = _narrow_phrase->rayCastManifold(_ray, {candidateId, position, quaternion, std::move(shape), std::move(collisionFilter)}))
            {
                const ColliderImpl::RigidbodyImpl& rigidbody = ref->instance<ColliderImpl::RigidbodyImpl>();
                _manifolds.emplace_back(raycast->distance(), raycast->normal(), sp<Rigidbody>::make(rigidbody.makeShadow()));
            }
    }

    const ColliderImpl::Stub& _collider;
    sp<NarrowPhrase> _narrow_phrase;
    NarrowPhrase::Ray _ray;

    Vector<RayCastManifold> _manifolds;
};

Vector<RayCastManifold> ColliderImpl::Stub::rayCast(const V3 from, const V3 to, const sp<CollisionFilter>& collisionFilter) const
{
    BroadPhraseCallbackRayCast broadPhraseCallback(*this, from, to);

    for(const auto& i : _broad_phrases | std::views::keys)
        i->rayCast(broadPhraseCallback, from, to, collisionFilter);

    return std::move(broadPhraseCallback._manifolds);
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
