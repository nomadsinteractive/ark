#pragma once

#include <Box2D/Box2D.h>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/components/rigidbody.h"
#include "app/inf/collider.h"
#include "app/inf/rigidbody_controller.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/collider_box2d.h"
#include "box2d/inf/shape.h"

namespace ark::plugin::box2d {

class ARK_PLUGIN_BOX2D_API RigidbodyBox2D final : public RigidbodyController {
public:
    struct Stub {
        Stub(const ColliderBox2D& world, b2Body* body);
        ~Stub();

        void dispose();
        b2Body* body();

        ColliderBox2D _world;
        b2Body* _body;
        OptionalVar<Boolean> _discarded;
        HashSet<RefId> _contacts;
    };

public:
    RigidbodyBox2D(const ColliderBox2D& world, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const OptionalVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, const sp<Shape>& shape, float density, float friction);
    RigidbodyBox2D(const ColliderBox2D& world, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const OptionalVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, const BodyCreateInfo& createInfo);
    RigidbodyBox2D(const sp<Stub>& stub, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const OptionalVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded = nullptr);

    const sp<Rigidbody::Stub>& rigidbodyStub() const;
    b2Body* body() const;

    float angle();
    void setAngle(float rad);

    V3 linearVelocity() const override;
    void setLinearVelocity(V3 velocity) override;

    V3 linearFactor() const override;
    void setLinearFactor(V3 factor) override;

    V3 angularVelocity() const override;
    void setAngularVelocity(V3 velocity) override;
    V3 angularFactor() const override;
    void setAngularFactor(V3 factor) override;

    void applyCentralImpulse(V3 impulse) override;
    float friction() const override;
    void setFriction(float friction) override;

    float mass() const override;
    void setMass(float mass) override;

    void applyCentralForce(V3 force) override;

    bool active() const override;
    void setActive(bool active) override;

    float gravityScale() const;
    void setGravityScale(float scale);

    bool awake();
    void setAwake(bool awake);

    float x() const;
    float y() const;

    void applyTorque(float torque, bool wake = true);
    void applyForce(const V2& force, const V2& point, bool wake = true);
    void applyLinearImpulse(const V2& impulse, const V2& point, bool wake = true);
    void applyAngularImpulse(float impulse, bool wake = true);

    void setTransform(const V2& position, float angle);

    sp<Future> applyLinearVelocity(const sp<Vec2>& velocity);
    sp<Future> applyRotate(const sp<Numeric>& rotate);

    Rigidbody makeShadow() const;

private:
    sp<Rigidbody::Stub> _rigidbody_stub;
    sp<Stub> _stub;

    friend class ColliderBox2D;
};

}
