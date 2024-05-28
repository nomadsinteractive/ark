#include "bullet/rigid_body/rigid_body_bullet.h"

#include "bullet/base/collision_shape.h"
#include "bullet/base/bt_rigid_body_ref.h"

namespace ark::plugin::bullet {

RigidBodyBullet::Stub::Stub(ColliderBullet world, sp<CollisionShape> collisionShape, sp<BtRigidBodyRef> rigidBody)
    : _world(std::move(world)), _collision_shape(std::move(collisionShape)), _rigid_body(std::move(rigidBody))
{
}

RigidBodyBullet::Stub::~Stub()
{
    btCollisionObject* collisionObject = _rigid_body->collisionObject();
    _world.btDynamicWorld()->removeCollisionObject(collisionObject);
    delete static_cast<WeakPtr<RigidBody::Stub>*>(collisionObject->getUserPointer());
    collisionObject->setUserPointer(nullptr);
    _rigid_body->reset();
}

RigidBodyBullet::RigidBodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> collisionShape, sp<Vec3> position, sp<Transform> transform, sp<BtRigidBodyRef> rigidBody)
    : RigidBody(sp<RigidBody::Stub>::make(id, type, 0, 0, std::move(position), nullptr, std::move(transform), sp<Stub>::make(std::move(world), std::move(collisionShape), std::move(rigidBody)))),
      _stub(stub()->_impl.toPtr<Stub>())
{
    _stub->_rigid_body->collisionObject()->setUserPointer(new WeakPtr<RigidBody::Stub>(stub()));
}

RigidBodyBullet::RigidBodyBullet(sp<RigidBody::Stub> other)
    : RigidBody(std::move(other)), _stub(stub()->_impl.toPtr<Stub>())
{
}

void RigidBodyBullet::dispose()
{
}

void RigidBodyBullet::applyCentralForce(const V3& force)
{
    _stub->_rigid_body->rigidBody()->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

V3 RigidBodyBullet::linearVelocity() const
{
    const btVector3& velocity = _stub->_rigid_body->rigidBody()->getLinearVelocity();
    return V3(velocity.x(), velocity.y(), velocity.z());
}

void RigidBodyBullet::setLinearVelocity(const V3& velocity)
{
    _stub->_rigid_body->rigidBody()->setActivationState(DISABLE_DEACTIVATION);
    _stub->_rigid_body->rigidBody()->setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));
}

float RigidBodyBullet::friction() const
{
    return _stub->_rigid_body->collisionObject()->getFriction();
}

void RigidBodyBullet::setFriction(float friction)
{
    _stub->_rigid_body->collisionObject()->setFriction(friction);
}

V3 RigidBodyBullet::angularFactor() const
{
    const btVector3& factor = _stub->_rigid_body->rigidBody()->getAngularFactor();
    return V3(factor.x(), factor.y(), factor.z());
}

void RigidBodyBullet::setAngularFactor(const V3& factor)
{
    _stub->_rigid_body->rigidBody()->setAngularFactor(btVector3(factor.x(), factor.y(), factor.z()));
}

const sp<BtRigidBodyRef>& RigidBodyBullet::rigidBody() const
{
    return _stub->_rigid_body;
}

}
