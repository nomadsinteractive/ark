#ifndef ARK_PLUGIN_BULLET_RIGID_BODY_RIGID_BODY_BULLET_H_
#define ARK_PLUGIN_BULLET_RIGID_BODY_RIGID_BODY_BULLET_H_

#include "core/base/timestamp.h"
#include "core/inf/variable.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "app/base/rigid_body.h"

#include "bullet/api.h"
#include "bullet/base/collider_bullet.h"
#include "bullet/forwarding.h"

namespace ark {
namespace plugin {
namespace bullet {

//[[script::bindings::extends(RigidBody)]]
//[[script::bindings::name("RigidBody")]]
class ARK_PLUGIN_BULLET_API RigidBodyBullet : public RigidBody, Implements<RigidBodyBullet, RigidBody, Holder>  {
public:
    RigidBodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass);
    RigidBodyBullet(const sp<RigidBody::Stub>& other);

    virtual void dispose() override;

//  [[script::bindings::auto]]
    void applyCentralForce(const V3& force);

//  [[script::bindings::property]]
    V3 linearVelocity() const;
//  [[script::bindings::property]]
    void setLinearVelocity(const V3& velocity);

//  [[script::bindings::property]]
    float friction() const;
//  [[script::bindings::property]]
    void setFriction(float friction);

//  [[script::bindings::property]]
    V3 angularFactor() const;
//  [[script::bindings::property]]
    void setAngularFactor(const V3& factor);

    const sp<BtRigidBodyRef>& rigidBody() const;

private:
    struct Stub;

    class Position : public Vec3 {
    public:
        Position(const sp<Stub>& stub);

        virtual bool update(uint64_t timestamp) override;

        virtual V3 val() override;

    private:
        V3 getWorldPosition() const;

    private:
        sp<Stub> _stub;
        bool _is_static_body;
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

    struct Stub {
        Stub(ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, Collider::BodyType bodyType, btScalar mass);
        ~Stub();

        btMotionState* makeMotionState(const btTransform& transform) const;
        sp<BtRigidBodyRef> makeRigidBody(btCollisionShape* shape, btMotionState* motionState, Collider::BodyType bodyType, btScalar mass) const;

        ColliderBullet _world;
        sp<CollisionShape> _shape;

        op<btMotionState> _motion_state;
        sp<BtRigidBodyRef> _rigid_body;

        Collider::BodyType _body_type;
    };

private:
    sp<Stub> _stub;
};

}
}
}

#endif
