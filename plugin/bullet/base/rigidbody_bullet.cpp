#include "bullet/base/rigidbody_bullet.h"

#include "app/traits/shape.h"

#include "bullet/base/collision_shape.h"
#include "bullet/base/bt_rigidbody_ref.h"

namespace ark::plugin::bullet {

RigidbodyBullet::Stub::Stub(ColliderBullet world, sp<CollisionShape> collisionShape, sp<BtRigidbodyRef> rigidBody)
    : _world(std::move(world)), _collision_shape(std::move(collisionShape)), _rigid_body(std::move(rigidBody))
{
}

RigidbodyBullet::Stub::~Stub()
{
    btCollisionObject* collisionObject = _rigid_body->collisionObject();
    _world.btDynamicWorld()->removeCollisionObject(collisionObject);
    collisionObject->setUserPointer(nullptr);
    _rigid_body->reset();
}

RigidbodyBullet::RigidbodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> collisionShape, sp<Vec3> position, sp<Transform> transform, sp<BtRigidbodyRef> rigidBody)
    : Rigidbody(type, nullptr, std::move(position), transform->rotation(), sp<Stub>::make(std::move(world), std::move(collisionShape), std::move(rigidBody)), nullptr),
      _stub(_impl.toPtr<Stub>())
{
    _stub->_rigid_body->collisionObject()->setUserPointer(this);
}

void RigidbodyBullet::applyCentralForce(const V3& force)
{
    _stub->_rigid_body->rigidBody()->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

V3 RigidbodyBullet::linearVelocity() const
{
    const btVector3& velocity = _stub->_rigid_body->rigidBody()->getLinearVelocity();
    return {velocity.x(), velocity.y(), velocity.z()};
}

void RigidbodyBullet::setLinearVelocity(const V3& velocity)
{
    _stub->_rigid_body->rigidBody()->setActivationState(DISABLE_DEACTIVATION);
    _stub->_rigid_body->rigidBody()->setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));
}

float RigidbodyBullet::friction() const
{
    return _stub->_rigid_body->collisionObject()->getFriction();
}

void RigidbodyBullet::setFriction(float friction)
{
    _stub->_rigid_body->collisionObject()->setFriction(friction);
}

V3 RigidbodyBullet::angularFactor() const
{
    const btVector3& factor = _stub->_rigid_body->rigidBody()->getAngularFactor();
    return V3(factor.x(), factor.y(), factor.z());
}

void RigidbodyBullet::setAngularFactor(const V3& factor)
{
    _stub->_rigid_body->rigidBody()->setAngularFactor(btVector3(factor.x(), factor.y(), factor.z()));
}

const sp<BtRigidbodyRef>& RigidbodyBullet::rigidBody() const
{
    return _stub->_rigid_body;
}

}
