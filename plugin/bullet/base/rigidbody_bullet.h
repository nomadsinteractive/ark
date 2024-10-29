#pragma once

#include "core/inf/variable.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "app/base/rigidbody.h"

#include "bullet/api.h"
#include "bullet/base/collider_bullet.h"
#include "bullet/forwarding.h"

namespace ark::plugin::bullet {

//[[script::bindings::extends(Rigidbody)]]
//[[script::bindings::name("Rigidbody")]]
class ARK_PLUGIN_BULLET_API RigidbodyBullet final : public Rigidbody, Implements<RigidbodyBullet, Rigidbody>  {
public:
    RigidbodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> collisionShape, sp<Vec3> position, sp<Vec4> quaternion, sp<BtRigidbodyRef> rigidBody);

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

    const sp<BtRigidbodyRef>& rigidBody() const;

private:
    struct Stub {
        Stub(ColliderBullet world, sp<CollisionShape> collisionShape, sp<BtRigidbodyRef> rigidBody);
        ~Stub();

        ColliderBullet _world;

        sp<CollisionShape> _collision_shape;
        sp<BtRigidbodyRef> _rigid_body;
    };

private:
    sp<Stub> _stub;
};

}
