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
    BtRigidBodyRef(btRigidBody* rigidBody);

    btRigidBody* ptr() const;

    void reset();

private:
    op<btRigidBody> _rigid_body;
};

}
}
}

#endif
