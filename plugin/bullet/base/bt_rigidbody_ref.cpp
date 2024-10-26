#include "bullet/base/bt_rigidbody_ref.h"

namespace ark::plugin::bullet {

BtRigidbodyRef::BtRigidbodyRef(btCollisionObject* rigidBody)
    : _rigid_body(rigidBody)
{
}

btRigidBody* BtRigidbodyRef::rigidBody() const
{
    btRigidBody* body = btRigidBody::upcast(_rigid_body.get());
    DCHECK(body, "CollisionObject(%d) is not a btRigidBody instance", _rigid_body->getInternalType());
    return body;
}

btCollisionObject* BtRigidbodyRef::collisionObject() const
{
    return _rigid_body.get();
}

void BtRigidbodyRef::reset()
{
    _rigid_body.reset();
}

}
