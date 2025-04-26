#include "bullet/base/bt_rigidbody_ref.h"

namespace ark::plugin::bullet {

BtRigidbodyRef::BtRigidbodyRef(btCollisionObject* collisionObject)
    : _collision_object(collisionObject), _destruction_count_down(2)
{
}

btRigidBody* BtRigidbodyRef::rigidBody() const
{
    btRigidBody* body = btRigidBody::upcast(_collision_object.get());
    CHECK(body, "CollisionObject(%d) is not a btRigidBody instance", _collision_object->getInternalType());
    return body;
}

btCollisionObject* BtRigidbodyRef::collisionObject() const
{
    return _collision_object.get();
}

bool BtRigidbodyRef::destroyCountDown(btDynamicsWorld* dynamicsWorld)
{
    ASSERT(_destruction_count_down >= 0);
    if(--_destruction_count_down <= 0)
    {
        _collision_object->setUserPointer(nullptr);
        dynamicsWorld->removeCollisionObject(_collision_object.get());
        _collision_object.reset();
        return true;
    }
    return false;
}

}
