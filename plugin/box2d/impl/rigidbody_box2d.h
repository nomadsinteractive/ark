#pragma once

#include <unordered_set>

#include <Box2D/Box2D.h>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/components/rigidbody.h"
#include "app/inf/collider.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/collider_box2d.h"
#include "box2d/inf/shape.h"

namespace ark::plugin::box2d {

//[[script::bindings::name("Rigidbody")]]
class ARK_PLUGIN_BOX2D_API RigidbodyBox2D {
public:
    struct Stub {
        Stub(const ColliderBox2D& world, b2Body* body);
        ~Stub();

        void dispose();
        b2Body* body();

        ColliderBox2D _world;
        b2Body* _body;
        SafeVar<Boolean> _discarded;
        std::unordered_set<RefId> _contacts;
    };

public:
//  [[script::bindings::auto]]
    RigidbodyBox2D(const ColliderBox2D& world, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const SafeVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, const sp<Shape>& shape, float density, float friction);
    RigidbodyBox2D(const ColliderBox2D& world, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const SafeVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, const BodyCreateInfo& createInfo);
    RigidbodyBox2D(const sp<Stub>& stub, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const SafeVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded = nullptr);

    const sp<Rigidbody::Stub>& rigidbodyStub() const;
    b2Body* body() const;

//  [[script::bindings::property]]
    float angle();
//  [[script::bindings::property]]
    void setAngle(float rad);

//  [[script::bindings::property]]
    float angularVelocity();
//  [[script::bindings::property]]
    void setAngularVelocity(float omega);

//  [[script::bindings::property]]
    V2 linearVelocity() const;
//  [[script::bindings::property]]
    void setLinearVelocity(const V2& velocity);

//  [[script::bindings::property]]
    float gravityScale() const;
//  [[script::bindings::property]]
    void setGravityScale(float scale);

//  [[script::bindings::property]]
    bool active();
//  [[script::bindings::property]]
    void setActive(bool active);

//  [[script::bindings::property]]
    bool awake();
//  [[script::bindings::property]]
    void setAwake(bool awake);

//  [[script::bindings::property]]
    float x() const;
//  [[script::bindings::property]]
    float y() const;
//  [[script::bindings::property]]
    float mass() const;

//  [[script::bindings::auto]]
    void applyTorque(float torque, bool wake = true);
//  [[script::bindings::auto]]
    void applyForce(const V2& force, const V2& point, bool wake = true);
//  [[script::bindings::auto]]
    void applyForceToCenter(const V2& force, bool wake = true);
//  [[script::bindings::auto]]
    void applyLinearImpulse(const V2& impulse, const V2& point, bool wake = true);
//  [[script::bindings::auto]]
    void applyAngularImpulse(float impulse, bool wake = true);

//  [[script::bindings::auto]]
    void setTransform(const V2& position, float angle);

//  [[script::bindings::auto]]
    sp<Future> applyLinearVelocity(const sp<Vec2>& velocity);
//  [[script::bindings::auto]]
    sp<Future> applyRotate(const sp<Numeric>& rotate);

    Rigidbody makeShadow() const;

private:
    sp<Rigidbody::Stub> _rigidbody_stub;
    sp<Stub> _stub;

    friend class ColliderBox2D;
};

}
