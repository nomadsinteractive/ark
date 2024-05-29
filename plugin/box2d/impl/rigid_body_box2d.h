#pragma once

#include <unordered_set>

#include <Box2D/Box2D.h>

#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "app/base/rigid_body.h"
#include "app/inf/collider.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/collider_box2d.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

//[[script::bindings::extends(RigidBody)]]
//[[script::bindings::name("RigidBody")]]
class ARK_PLUGIN_BOX2D_API RigidBodyBox2D : public RigidBody, Implements<RigidBodyBox2D, RigidBody> {
public:
    struct Stub {
        Stub(const ColliderBox2D& world, b2Body* body);
        ~Stub();

        void dispose();

        b2Body* body();

        int32_t _id;
        ColliderBox2D _world;
        b2Body* _body;

        SafePtr<Boolean> _disposed;
        sp<RigidBody::Callback> _callback;

        std::unordered_set<int32_t> _contacts;
    };

    struct Shadow {
        Shadow(const sp<Stub>& body, const sp<RigidBody::Stub>& rigidBody);

        WeakPtr<Stub> _body;
        WeakPtr<RigidBody::Stub> _rigid_body;
        Box _tag;
    };

public:
//  [[script::bindings::auto]]
    RigidBodyBox2D(const ColliderBox2D& world, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Numeric>& rotate, const sp<Shape>& shape, float density, float friction);
    RigidBodyBox2D(const ColliderBox2D& world, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Numeric>& rotate, const BodyCreateInfo& createInfo);
    RigidBodyBox2D(const sp<Stub>& stub, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Numeric>& rotate);
    RigidBodyBox2D(const sp<Stub>& stub, const sp<RigidBody::Stub>& rigidbody);

    void bind(const sp<RenderObject>& renderObject) override;
    void dispose() override;

    static sp<RigidBodyBox2D> obtain(const Shadow* shadow);

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

private:
    sp<Stub> _stub;

    friend class ColliderBox2D;
};

}
}
}
