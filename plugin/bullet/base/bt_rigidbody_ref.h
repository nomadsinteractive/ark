#pragma once

#include "core/types/owned_ptr.h"

#include "bullet/forwarding.h"

#include "btBulletDynamicsCommon.h"


namespace ark::plugin::bullet {

class BtRigidbodyRef  {
public:
    BtRigidbodyRef(btCollisionObject* rigidBody);

    btRigidBody* rigidBody() const;
    btCollisionObject* collisionObject() const;

    void reset();

private:
    op<btCollisionObject> _rigid_body;
};

}
