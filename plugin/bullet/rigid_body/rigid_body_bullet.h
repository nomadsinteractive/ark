#ifndef ARK_PLUGIN_BULLET_RIGID_BODY_RIGID_BODY_BULLET_H_
#define ARK_PLUGIN_BULLET_RIGID_BODY_RIGID_BODY_BULLET_H_

#include "core/base/timestamp.h"
#include "core/inf/variable.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

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
    class Stub;

    class Position : public Vec3 {
    public:
        Position(const sp<Stub>& stub);

        virtual bool update(uint64_t timestamp) override;

        virtual V3 val() override;

    private:
        V3 getWorldPosition() const;

    private:
        sp<Stub> _stub;
    };

    class TransformDelegate : public Transform::Delegate {
    public:
        TransformDelegate(const sp<Stub>& stub);

        virtual void snapshot(const Transform& transform, Transform::Snapshot& snapshot) const override;
        virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
        virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

    private:
        sp<Stub> _stub;
    };

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

        friend class Position;
        friend class TransformDelegate;
    };

private:
    sp<Stub> _stub;
};

}
}
}

#endif
