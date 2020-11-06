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
    RigidBodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> collisionShape, sp<Vec3> position, sp<Transform> transform, sp<BtRigidBodyRef> rigidBody);
    RigidBodyBullet(sp<Stub> other);

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
    struct Stub {
        Stub(ColliderBullet world, sp<CollisionShape> collisionShape, sp<BtRigidBodyRef> rigidBody);
        ~Stub();

        ColliderBullet _world;

        sp<CollisionShape> _collision_shape;
        sp<BtRigidBodyRef> _rigid_body;
    };

private:
    sp<Stub> _stub;
};

}
}
}

#endif
