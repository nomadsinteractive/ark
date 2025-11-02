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

    bool active() const override;
    void setActive(bool active) override;

    V3 centralForce() const override;
    void setCentralForce(const V3& force) override;

    V3 linearVelocity() const override;
    void setLinearVelocity(const V3& velocity) override;

    V3 linearFactor() const override;
    void setLinearFactor(const V3& factor) override;

    V3 angularVelocity() const override;
    void setAngularVelocity(const V3& velocity) override;

    V3 angularFactor() const override;
    void setAngularFactor(const V3& factor) override;

    void applyCentralForce(const V3& force) override;
    void applyCentralImpulse(const V3& impulse) override;

    float friction() const override;
    void setFriction(float friction) override;

    float mass() const override;
    void setMass(float mass) override;

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
