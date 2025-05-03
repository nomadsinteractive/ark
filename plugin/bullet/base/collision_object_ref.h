#pragma once

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "bullet/forwarding.h"

#include "btBulletDynamicsCommon.h"


namespace ark::plugin::bullet {

class CollisionObjectRef  {
public:
    CollisionObjectRef(btCollisionObject* collisionObject, sp<CollisionShapeRef> collisionShape, sp<btMotionState> motionState = nullptr);

    btRigidBody* rigidBody() const;
    btCollisionObject* collisionObject() const;

    bool destroyCountDown(btDynamicsWorld* dynamicsWorld);

private:
    op<btCollisionObject> _collision_object;
    sp<CollisionShapeRef> _collision_shape;
    sp<btMotionState> _motion_state;
};

}
