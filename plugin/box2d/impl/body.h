#ifndef ARK_PLUGIN_BOX2D_IMPL_BODY_H_
#define ARK_PLUGIN_BOX2D_IMPL_BODY_H_

#include <Box2D/Box2D.h>

#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "app/base/rigid_body.h"
#include "app/inf/collider.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/world.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

//[[script::bindings::holder]]
//[[script::bindings::extends(RigidBody)]]
class ARK_PLUGIN_BOX2D_API Body : public RigidBody, Implements<Body, RigidBody, Holder> {
public:
    struct Stub {
        Stub(const World& world, b2Body* body);
        ~Stub();

        void dispose();

        int32_t _id;
        World _world;
        b2Body* _body;

        SafePtr<Disposed> _disposed;
        sp<RigidBody::Callback> _callback;

        std::unordered_set<int32_t> _contacts;
    };

    struct Shadow {
        Shadow(const sp<Stub>& body, const sp<RigidBody::Stub>& rigidBody);

        WeakPtr<Stub> _body;
        WeakPtr<RigidBody::Stub> _rigid_body;
        sp<Box> _tag;
    };

public:
//  [[script::bindings::auto]]
    Body(const World& world, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotate, const sp<Shape>& shape, float density, float friction, bool isSensor = false);
    Body(const World& world, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotate, const BodyCreateInfo& createInfo);
    Body(const sp<Stub>& stub, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotate);
    Body(const sp<Stub>& stub, const sp<RigidBody::Stub>& rigidbody);

    virtual void bind(const sp<RenderObject>& renderObject) override;
    virtual void dispose() override;

    static sp<Body> obtain(const Shadow* shadow, ObjectPool& objectPool);

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

    friend class World;
};

}
}
}

#endif
