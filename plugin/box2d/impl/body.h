#ifndef ARK_PLUGIN_BOX2D_IMPL_BODY_H_
#define ARK_PLUGIN_BOX2D_IMPL_BODY_H_

#include <Box2D/Box2D.h>

#include "core/base/object.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/base/rigid_body.h"
#include "app/inf/collider.h"

#include "box2d/api.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class World;

class ARK_PLUGIN_BOX2D_API Body : public Object, public RigidBody, Implements<Body, Object, RigidBody> {
public:
//  [[script::bindings::auto]]
    Body(const sp<World>& world, Collider::BodyType type, float x, float y, Shape& shape, float density, float friction);
    Body(const sp<World>& world, b2Body* body);
    ~Body();

    virtual void dispose() override;
    virtual const sp<CollisionCallback>& collisionCallback() const override;
    virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

//  [[script::bindings::auto]]
    const sp<World>& world() const;

//  [[script::bindings::property]]
    float rotation();
//  [[script::bindings::property]]
    void setRotation(float rad);

//  [[script::bindings::property]]
    float angularVelocity();
//  [[script::bindings::property]]
    void setAngularVelocity(float omega);

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
    void applyTorque(float torque);
//  [[script::bindings::auto]]
    void applyForce(const sp<VV>& force, const sp<VV>& point);
//  [[script::bindings::auto]]
    void applyForceToCenter(const sp<VV>& force);
//  [[script::bindings::auto]]
    void applyLinearImpulse(const sp<VV>& impulse, const sp<VV>& point);
//  [[script::bindings::auto]]
    void applyAngularImpulse(float impulse);

    b2Body* b2Instance() const;

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<Body> {
    public:
        BUILDER_IMPL1(BeanFactory& parent, const document& doc);

        virtual sp<Body> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Object>> _world;
        sp<Builder<Shape>> _shape;
        sp<Builder<VV>> _position;
        float _density;
        float _friction;

    };

//  [[plugin::builder("b2Body")]]
    class BUILDER_IMPL2 : public Builder<Object> {
    public:
        BUILDER_IMPL2(BeanFactory& parent, const document& doc);

        virtual sp<Object> build(const sp<Scope>& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

//  [[plugin::builder("b2Position")]]
    class POSITION_BUILDER : public Builder<Vec> {
    public:
        POSITION_BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Vec> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Object>> _body;
    };

//  [[plugin::builder("b2Rotation")]]
    class ROTATION_BUILDER : public Builder<Numeric> {
    public:
        ROTATION_BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Object>> _body;
    };

//  [[plugin::style("b2-body")]]
    class RENDER_OBJECT_STYLE : public Builder<RenderObject> {
    public:
        RENDER_OBJECT_STYLE(BeanFactory& parent, const sp<Builder<RenderObject>>& delegate, const String& value);

        virtual sp<RenderObject> build(const sp<Scope>& args) override;

    private:
        sp<Builder<RenderObject>> _delegate;
        sp<Builder<Object>> _body;

    };

    struct Stub {
        Stub(const sp<World>& world, b2Body* body);

        sp<World> _world;
        b2Body* _body;

        sp<CollisionCallback> _collision_callback;
    };

private:
    Body(const sp<Stub>& stub);

    sp<Stub> _stub;
};

}
}
}

#endif
