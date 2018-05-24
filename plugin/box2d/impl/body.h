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
#include "box2d/impl/world.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class World;

class ARK_PLUGIN_BOX2D_API Body : public Object, public RigidBody, Implements<Body, Object, RigidBody> {
public:
//  [[script::bindings::auto]]
    Body(const World& world, Collider::BodyType type, float x, float y, const sp<Size>& size, Shape& shape, float density, float friction);

    virtual void bind(const sp<RenderObject>& renderObject) override;
    virtual void dispose() override;

    const World& world() const;

/*
//  [[script::bindings::auto]]
    virtual void dispose() = 0;
//  [[script::bindings::auto]]
    virtual void bind(const sp<RenderObject>& renderObject);

//  [[script::bindings::property]]
    int32_t id() const;
//  [[script::bindings::property]]
    Collider::BodyType type() const;

//  [[script::bindings::property]]
    V xy() const;
//  [[script::bindings::property]]
    V3 xyz() const;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;

//  [[script::bindings::property]]
    const sp<Vec>& position() const;
//  [[script::bindings::property]]
    const sp<Size>& size() const;
//  [[script::bindings::property]]
    const sp<Rotate>& rotate() const;

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& box) const;

//  [[script::bindings::property]]
    const sp<CollisionCallback>& collisionCallback() const;
//  [[script::bindings::property]]
    void setCollisionCallback(const sp<CollisionCallback>& collisionCallback);
*/

//  [[script::bindings::property]]
    float rotation();
//  [[script::bindings::property]]
    void setRotation(float rad);

//  [[script::bindings::property]]
    float angularVelocity();
//  [[script::bindings::property]]
    void setAngularVelocity(float omega);

//  [[script::bindings::property]]
    V2 linearVelocity() const;
//  [[script::bindings::property]]
    void setLinearVelocity(const V2& velocity);

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
    void applyForce(const V2& force, const V2& point);
//  [[script::bindings::auto]]
    void applyForceToCenter(const V2& force);
//  [[script::bindings::auto]]
    void applyLinearImpulse(const V2& impulse, const V2& point);
//  [[script::bindings::auto]]
    void applyAngularImpulse(float impulse);

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<Body> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest);

        virtual sp<Body> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Object>> _world;
        sp<Builder<Shape>> _shape;
        sp<Builder<Vec>> _position;
        sp<Builder<Size>> _size;
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

    struct Stub {
        Stub(const World& world, b2Body* body);
        ~Stub();

        void dispose();

        World _world;
        int32_t _id;
        b2Body* _body;
    };

private:
    Body(const sp<Stub>& stub, Collider::BodyType type, const sp<Size>& size);

    sp<Stub> _stub;
};

}
}
}

#endif
