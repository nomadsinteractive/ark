#include "bullet/base/bt_rigid_body_ref.h"

namespace ark {
namespace plugin {
namespace bullet {

BtRigidBodyRef::BtRigidBodyRef(btCollisionObject* rigidBody)
    : _rigid_body(rigidBody)
{
}

btRigidBody* BtRigidBodyRef::rigidBody() const
{
    btRigidBody* body = btRigidBody::upcast(_rigid_body.get());
    DCHECK(body, "CollisionObject(%d) is not a btRigidBody instance", _rigid_body->getInternalType());
    return body;
}

btCollisionObject* BtRigidBodyRef::collisionObject() const
{
    return _rigid_body.get();
}

void BtRigidBodyRef::reset()
{
    _rigid_body.reset();
}

}
}
}
