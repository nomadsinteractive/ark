#ifndef ARK_PLUGIN_BULLET_BASE_BT_RIGID_BODY_REF_H_
#define ARK_PLUGIN_BULLET_BASE_BT_RIGID_BODY_REF_H_

#include "core/types/owned_ptr.h"

#include "bullet/forwarding.h"

#include "btBulletDynamicsCommon.h"


namespace ark {
namespace plugin {
namespace bullet {

class BtRigidBodyRef  {
public:
    BtRigidBodyRef(btCollisionObject* rigidBody);

    btRigidBody* rigidBody() const;
    btCollisionObject* collisionObject() const;

    void reset();

private:
    op<btCollisionObject> _rigid_body;
};

}
}
}

#endif
