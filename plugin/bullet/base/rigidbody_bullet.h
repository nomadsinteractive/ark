#pragma once

#include "core/inf/variable.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/components/rigidbody.h"
#include "app/inf/rigidbody_controller.h"

#include "bullet/api.h"
#include "bullet/base/collider_bullet.h"
#include "bullet/forwarding.h"

namespace ark::plugin::bullet {

class ARK_PLUGIN_BULLET_API RigidbodyBullet final : public RigidbodyController {
public:
    RigidbodyBullet(ColliderBullet& world, sp<CollisionObjectRef> rigidBody, Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded);

    bool validate() const;
    bool unique() const;

    void applyCentralForce(const V3& force);

    V3 linearVelocity() const override;
    void setLinearVelocity(const V3& velocity) override;

    void applyCentralImpulse(const V3& impulse) override;

    float friction() const;
    void setFriction(float friction);

    V3 angularFactor() const;
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
        bool _marked_for_destroy;
    };

    RigidbodyBullet(sp<Stub> stub);

private:
    sp<Stub> _stub;

    friend class ColliderBullet;
};

}
