#ifndef ARK_PLUGIN_BULLET_RIGID_BODY_RIGID_BODY_BULLET_H_
#define ARK_PLUGIN_BULLET_RIGID_BODY_RIGID_BODY_BULLET_H_

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "app/base/rigid_body.h"

#include "bullet/base/collider_bullet.h"
#include "bullet/forwarding.h"

namespace ark {
namespace plugin {
namespace bullet {

class RigidBodyBullet : public RigidBody {
public:
    RigidBodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass);

    virtual void dispose() override;

private:
    class Stub {
    public:
        Stub(ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass);
        ~Stub();

    private:
        btMotionState* makeMotionState(const btTransform& transform) const;
        btRigidBody* makeRigidBody(btCollisionShape* shape, btMotionState* motionState, btScalar mass) const;

    private:
        ColliderBullet _world;
        sp<CollisionShape> _shape;

        op<btMotionState> _motion_state;
        op<btRigidBody> _rigid_body;

        btVector3 _local_inertia;
    };

private:
    sp<Stub> _stub;
};

}
}
}

#endif
