#include "bullet/base/collision_object_ref.h"

#include "rigidbody_bullet.h"

namespace ark::plugin::bullet {

CollisionObjectRef::CollisionObjectRef(btCollisionObject* collisionObject, sp<CollisionShapeRef> collisionShape)
    : _collision_object(collisionObject), _collision_shape(std::move(collisionShape))
{
    _collision_object->setUserIndex(2);
}

btRigidBody* CollisionObjectRef::rigidBody() const
{
    btRigidBody* body = btRigidBody::upcast(_collision_object.get());
    CHECK(body, "CollisionObject(%d) is not a btRigidBody instance", _collision_object->getInternalType());
    return body;
}

btCollisionObject* CollisionObjectRef::collisionObject() const
{
    return _collision_object.get();
}

bool CollisionObjectRef::destroyCountDown(btDynamicsWorld* dynamicsWorld)
{
    const int32_t countDown = _collision_object->getUserIndex() - 1;
    if(countDown <= 0)
    {
        RigidbodyBullet::releaseCollisionObjectPointer(_collision_object->getUserPointer());
        dynamicsWorld->removeCollisionObject(_collision_object.get());
        _collision_object->setUserPointer(nullptr);
        _collision_object.reset();
        return true;
    }
    _collision_object->setUserIndex(countDown);
    return false;
}

}
