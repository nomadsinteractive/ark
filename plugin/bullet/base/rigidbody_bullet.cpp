#include "bullet/base/rigidbody_bullet.h"

#include "core/base/ref_manager.h"
#include "core/types/global.h"

#include "app/components/shape.h"

#include "bullet/base/collision_shape.h"
#include "bullet/base/bt_rigidbody_ref.h"

namespace ark::plugin::bullet {

RigidbodyBullet::Stub::Stub(ColliderBullet& world, sp<CollisionShape> collisionShape, sp<BtRigidbodyRef> rigidBody)
    : _world(world), _collision_shape(std::move(collisionShape)), _rigidbody(std::move(rigidBody))
{
}

RigidbodyBullet::Stub::~Stub()
{
    RigidbodyBullet* rigidbodyBullet = static_cast<RigidbodyBullet*>(_rigidbody->collisionObject()->getUserPointer());
    _world.markForDestroy(*rigidbodyBullet);
}

RigidbodyBullet::RigidbodyBullet(ColliderBullet& world, sp<BtRigidbodyRef> rigidBody, Rigidbody::BodyType type, sp<Shape> shape, sp<CollisionShape> collisionShape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
    : _rigidbody_stub(sp<Rigidbody::Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter))), _bt_rigidbody_stub(sp<Stub>::make(std::move(world), std::move(collisionShape), std::move(rigidBody)))
{
    _bt_rigidbody_stub->_rigidbody->collisionObject()->setUserPointer(this);
}

void RigidbodyBullet::discard()
{
    _rigidbody_stub->_ref->discard();
    _bt_rigidbody_stub->_world.markForDestroy(*this);
}

const sp<Ref>& RigidbodyBullet::ref() const
{
    return _rigidbody_stub->_ref;
}

const sp<Rigidbody::Stub>& RigidbodyBullet::stub() const
{
    return _rigidbody_stub;
}

const sp<CollisionCallback>& RigidbodyBullet::collisionCallback() const
{
    return _rigidbody_stub->_collision_callback;
}

Rigidbody RigidbodyBullet::makeShadow() const
{
    return {{_rigidbody_stub, nullptr}, true};
}

void RigidbodyBullet::applyCentralForce(const V3& force)
{
    _bt_rigidbody_stub->_rigidbody->rigidBody()->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

V3 RigidbodyBullet::linearVelocity() const
{
    const btVector3& velocity = _bt_rigidbody_stub->_rigidbody->rigidBody()->getLinearVelocity();
    return {velocity.x(), velocity.y(), velocity.z()};
}

void RigidbodyBullet::setLinearVelocity(const V3& velocity)
{
    _bt_rigidbody_stub->_rigidbody->rigidBody()->setActivationState(DISABLE_DEACTIVATION);
    _bt_rigidbody_stub->_rigidbody->rigidBody()->setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));
}

float RigidbodyBullet::friction() const
{
    return _bt_rigidbody_stub->_rigidbody->collisionObject()->getFriction();
}

void RigidbodyBullet::setFriction(float friction)
{
    _bt_rigidbody_stub->_rigidbody->collisionObject()->setFriction(friction);
}

V3 RigidbodyBullet::angularFactor() const
{
    const btVector3& factor = _bt_rigidbody_stub->_rigidbody->rigidBody()->getAngularFactor();
    return {factor.x(), factor.y(), factor.z()};
}

void RigidbodyBullet::setAngularFactor(const V3& factor)
{
    _bt_rigidbody_stub->_rigidbody->rigidBody()->setAngularFactor(btVector3(factor.x(), factor.y(), factor.z()));
}

const sp<BtRigidbodyRef>& RigidbodyBullet::btRigidbodyRef() const
{
    return _bt_rigidbody_stub->_rigidbody;
}

}
