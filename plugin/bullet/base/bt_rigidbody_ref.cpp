#include "bullet/base/bt_rigidbody_ref.h"

namespace ark::plugin::bullet {

BtRigidbodyRef::BtRigidbodyRef(btCollisionObject* collisionObject)
    : _collision_object(collisionObject)
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

void BtRigidbodyRef::reset()
{
    _collision_object.reset();
}

}
