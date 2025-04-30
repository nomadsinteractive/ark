#pragma once

#include "core/inf/variable.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/components/rigidbody.h"

#include "bullet/api.h"
#include "bullet/base/collider_bullet.h"
#include "bullet/forwarding.h"

namespace ark::plugin::bullet {

class ARK_PLUGIN_BULLET_API RigidbodyBullet final  {
public:
    RigidbodyBullet(ColliderBullet& world, sp<CollisionObjectRef> rigidBody, Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded);

    bool validate();

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

    const sp<Ref>& ref() const;
    const sp<Rigidbody::Stub>& stub() const;

    Rigidbody makeShadow() const;
    const sp<CollisionCallback>& collisionCallback() const;
    const sp<CollisionObjectRef>& collisionObjectRef() const;

    static RigidbodyBullet fromCollisionObjectPointer(void* ptr);
    static void releaseCollisionObjectPointer(void* ptr);

private:
    struct Stub {
        Stub(ColliderBullet& world, sp<Rigidbody::Stub> rigidbodyStub, sp<CollisionObjectRef> rigidBody);
        ~Stub();

        void markForDestroy();

        ColliderBullet& _world;

        sp<Rigidbody::Stub> _rigidbody_stub;
        sp<CollisionObjectRef> _collision_object_ref;
    };

    RigidbodyBullet(sp<Stub> stub);

private:
    sp<Stub> _stub;

    friend class ColliderBullet;
};

}
