#include "bullet/base/rigidbody_bullet.h"

#include "core/base/ref_manager.h"
#include "core/types/global.h"

#include "app/components/shape.h"

#include "bullet/base/collision_shape_ref.h"
#include "bullet/base/collision_object_ref.h"

namespace ark::plugin::bullet {

RigidbodyBullet::Stub::Stub(ColliderBullet& world, sp<Rigidbody::Stub> rigidbodyStub, sp<CollisionObjectRef> rigidBody)
    : _world(world), _rigidbody_stub(std::move(rigidbodyStub)), _collision_object_ref(std::move(rigidBody))
{
}

RigidbodyBullet::Stub::~Stub()
{
    markForDestroy();
}

void RigidbodyBullet::Stub::markForDestroy()
{
    if(_collision_object_ref)
    {
        _collision_object_ref->collisionObject()->setUserPointer(new Stub(_world, _rigidbody_stub, nullptr));
        _world.markForDestroy(std::move(_collision_object_ref));
    }
}

RigidbodyBullet::RigidbodyBullet(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

RigidbodyBullet::RigidbodyBullet(ColliderBullet& world, sp<CollisionObjectRef> rigidBody, Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
    : _stub(sp<Stub>::make(world, sp<Rigidbody::Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter)), std::move(rigidBody)))
{
    _stub->_collision_object_ref->collisionObject()->setUserPointer(_stub.get());
}

bool RigidbodyBullet::validate() const
{
    if(!(_stub && _stub->_collision_object_ref))
        return false;

    if(_stub->_rigidbody_stub->_ref->isDiscarded())
    {
        _stub->markForDestroy();
        return false;
    }
    return true;
}

bool RigidbodyBullet::unique() const
{
    return _stub.unique();
}

const sp<Ref>& RigidbodyBullet::ref() const
{
    return _stub->_rigidbody_stub->_ref;
}

const sp<Rigidbody::Stub>& RigidbodyBullet::stub() const
{
    return _stub->_rigidbody_stub;
}

const sp<CollisionCallback>& RigidbodyBullet::collisionCallback() const
{
    return _stub->_rigidbody_stub->_collision_callback;
}

Rigidbody RigidbodyBullet::makeShadow() const
{
    return {{_stub->_rigidbody_stub, nullptr}, true};
}

void RigidbodyBullet::applyCentralForce(const V3& force)
{
    _stub->_collision_object_ref->rigidBody()->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

V3 RigidbodyBullet::linearVelocity() const
{
    const btVector3& velocity = _stub->_collision_object_ref->rigidBody()->getLinearVelocity();
    return {velocity.x(), velocity.y(), velocity.z()};
}

void RigidbodyBullet::setLinearVelocity(const V3& velocity)
{
    _stub->_collision_object_ref->rigidBody()->setActivationState(DISABLE_DEACTIVATION);
    _stub->_collision_object_ref->rigidBody()->setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));
}

float RigidbodyBullet::friction() const
{
    return _stub->_collision_object_ref->collisionObject()->getFriction();
}

void RigidbodyBullet::setFriction(const float friction)
{
    _stub->_collision_object_ref->collisionObject()->setFriction(friction);
}

V3 RigidbodyBullet::angularFactor() const
{
    const btVector3& factor = _stub->_collision_object_ref->rigidBody()->getAngularFactor();
    return {factor.x(), factor.y(), factor.z()};
}

void RigidbodyBullet::setAngularFactor(const V3& factor)
{
    _stub->_collision_object_ref->rigidBody()->setAngularFactor(btVector3(factor.x(), factor.y(), factor.z()));
}

const sp<CollisionObjectRef>& RigidbodyBullet::collisionObjectRef() const
{
    return _stub->_collision_object_ref;
}

RigidbodyBullet RigidbodyBullet::fromCollisionObjectPointer(void* ptr)
{
    if(ptr)
        return RigidbodyBullet(sp<Stub>(std::shared_ptr<Stub>(static_cast<Stub*>(ptr), [](RigidbodyBullet::Stub*) {}), Class::ensureClass<Stub>()));
    return {nullptr};
}

void RigidbodyBullet::releaseCollisionObjectPointer(void* ptr)
{
    delete static_cast<Stub*>(ptr);
}

}
