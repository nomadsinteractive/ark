#include "bullet/base/bt_rigid_body_ref.h"

namespace ark {
namespace plugin {
namespace bullet {

BtRigidBodyRef::BtRigidBodyRef(btRigidBody* rigidBody)
    : _rigid_body(rigidBody)
{
}

btRigidBody* BtRigidBodyRef::ptr() const
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
