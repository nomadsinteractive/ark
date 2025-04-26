#pragma once

#include "core/types/owned_ptr.h"

#include "bullet/forwarding.h"

#include "btBulletDynamicsCommon.h"


namespace ark::plugin::bullet {

class BtRigidbodyRef  {
public:
    BtRigidbodyRef(btCollisionObject* collisionObject);

    btRigidBody* rigidBody() const;
    btCollisionObject* collisionObject() const;

    bool markForDestroy();
    bool destroyCountDown(btDynamicsWorld* dynamicsWorld);

private:
    op<btCollisionObject> _collision_object;
    int32_t _destruction_count_down;
};

}
