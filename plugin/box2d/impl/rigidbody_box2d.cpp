#include "box2d/impl/rigidbody_box2d.h"

#include "core/ark.h"
#include "core/base/future.h"
#include "core/base/ref_manager.h"
#include "core/inf/variable.h"
#include "core/types/global.h"

#include "graphics/base/rect.h"
#include "graphics/components/render_object.h"
#include "graphics/components/rotation.h"
#include "graphics/base/v3.h"

#include "app/base/application_context.h"
#include "graphics/base/shape.h"

#include "box2d/impl/collider_box2d.h"
#include "graphics/base/rotation_axis_theta.h"

namespace ark::plugin::box2d {

namespace {

class _RigidBodyRotation final : public Numeric {
public:
    _RigidBodyRotation(const sp<RigidbodyBox2D::Stub>& stub)
        : _stub(stub) {
    }

    float val() override {
        DCHECK(b2Body_IsValid(_stub->_body), "Body has been disposed already");
        return b2Rot_GetAngle(b2Body_GetTransform(_stub->_body).q);
    }

    bool update(uint32_t /*tick*/) override {
        return true;
    }

    sp<RigidbodyBox2D::Stub> _stub;
};

class _RigidBodyPosition : public Vec3 {
public:
    _RigidBodyPosition(const sp<RigidbodyBox2D::Stub>& stub, const sp<Vec3>& delegate)
        : _stub(stub)/*, _delegate(delegate)*/ {
    }

    V3 val() override {
        DCHECK(b2Body_IsValid(_stub->_body), "Body has been disposed already");
        const b2Transform transform = b2Body_GetTransform(_stub->_body);
        return {transform.p.x, transform.p.y, 0};
    }

    bool update(uint32_t /*tick*/) override {
        return true;
    }

    sp<RigidbodyBox2D::Stub> _stub;
    sp<Vec3> _delegate;

};

class RenderObjectPosition : public Vec3 {
public:
    RenderObjectPosition(const sp<RigidbodyBox2D::Stub>& stub)
        : _stub(stub) {
    }

    V3 val() override {
        DCHECK(b2Body_IsValid(_stub->_body), "Body has been disposed already");
        const b2Transform transform = b2Body_GetTransform(_stub->_body);
        const float x = _stub->_world.toPixelX(transform.p.x);
        const float y = _stub->_world.toPixelY(transform.p.y);
        return {x, y, 0};
    }

    bool update(uint32_t /*tick*/) override {
        return true;
    }

private:
    sp<RigidbodyBox2D::Stub> _stub;
};

class ManualLinearVelocity : public Runnable {
public:
    ManualLinearVelocity(const sp<RigidbodyBox2D::Stub>& body, const sp<Vec2>& velocity, const sp<Future>& future)
        : _body(body), _velocity(velocity), _future(future) {
    }

    void run() override {
        const V2 v = _velocity->val();
        if(b2Body_IsValid(_body->_body))
        {
            b2Body_SetLinearVelocity(_body->_body, b2Vec2(v.x(), v.y()));
        }
        else if(_future)
        {
            _future->cancel();
            _future = nullptr;
            _body = nullptr;
        }
    }

private:
    sp<RigidbodyBox2D::Stub> _body;
    sp<Vec2> _velocity;
    sp<Future> _future;
};

class ManualApplyRotate : public Runnable {
public:
    ManualApplyRotate(const sp<RigidbodyBox2D::Stub>& body, const sp<Numeric>& rotate, const sp<Future>& future)
        : _body(body), _rotate(rotate), _future(future) {
    }

    void run() override {
        if(b2Body_IsValid(_body->_body))
        {
            const float rotation = _rotate->val();
            const b2Transform transform = {b2Body_GetPosition(_body->_body), b2MakeRot(rotation)};
            b2Body_SetTargetTransform(_body->_body, transform, 0.01);
        }
        else if(_future)
        {
            _future->cancel();
            _future = nullptr;
            _body = nullptr;
        }
    }

private:
    sp<RigidbodyBox2D::Stub> _body;
    sp<Numeric> _rotate;
    sp<Future> _future;
};

class BodyDisposer {
public:
    BodyDisposer(const ColliderBox2D& world, b2BodyId body)
        : _world(world), _body_id(body) {
    }
    ~BodyDisposer() {
        DCHECK(b2Body_IsValid(_body_id), "Body has been disposed already");
        b2DestroyBody(_body_id);
    }

private:
    ColliderBox2D _world;
    b2BodyId _body_id;
};

}

RigidbodyBox2D::RigidbodyBox2D(const ColliderBox2D& world, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const OptionalVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, const sp<Shape>& shape, float density, float friction)
    : RigidbodyBox2D(world, type, position, size, rotate, std::move(collisionFilter), BodyCreateInfo(shape, density, friction, type & Rigidbody::BODY_TYPE_SENSOR))
{
}

RigidbodyBox2D::RigidbodyBox2D(const ColliderBox2D& world, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const OptionalVar<Numeric>& rotate, sp<CollisionFilter> collisionFilter, const BodyCreateInfo& createInfo)
    : RigidbodyBox2D(sp<Stub>::make(world, world.createBody(type, position->val(), size, createInfo)), type, position, size, rotate, std::move(collisionFilter))
{
}

//TODO: Manual rotation
RigidbodyBox2D::RigidbodyBox2D(const sp<Stub>& stub, Rigidbody::BodyType type, const sp<Vec3>& position, const V3& size, const OptionalVar<Numeric>& rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
    : _rigidbody_stub(sp<Rigidbody::Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, sp<ark::Shape>::make(0, size), sp<_RigidBodyPosition>::make(stub, position), sp<Vec4>::make<RotationAxisTheta>(sp<Vec3>::make<Vec3::Const>(V3(constants::AXIS_Z)), sp<Numeric>::make<_RigidBodyRotation>(stub)), std::move(collisionFilter))),
      _stub(stub)
{
    b2Body_SetUserData(_stub->_body, this);
}

const sp<Rigidbody::Stub>& RigidbodyBox2D::rigidbodyStub() const
{
    return _rigidbody_stub;
}

b2BodyId RigidbodyBox2D::body() const
{
    return _stub->body();
}

float RigidbodyBox2D::angle()
{
    FATAL("Unimplemented");
    return 0;
}

void RigidbodyBox2D::setAngle(float rad)
{
    FATAL("Unimplemented");
}

V3 RigidbodyBox2D::position() const
{
    return _rigidbody_stub->_position.val();
}

void RigidbodyBox2D::setPosition(const V3& position)
{
    FATAL("Unimplemented");
}

V3 RigidbodyBox2D::centralForce() const
{
    return V3(0);
}

void RigidbodyBox2D::setCentralForce(const V3& force)
{
    b2Body_ApplyForceToCenter(_stub->body(), {force.x(), force.y()}, true);
}

V3 RigidbodyBox2D::linearVelocity() const
{
    const b2Vec2 velocity = b2Body_GetLinearVelocity(_stub->body());
    return {velocity.x, velocity.y, 0};
}

V3 RigidbodyBox2D::angularVelocity() const
{
    return V3(0, 0, b2Body_GetAngularVelocity(_stub->body()));
}

void RigidbodyBox2D::setAngularVelocity(const V3& velocity)
{
    b2Body_SetAngularVelocity(_stub->body(), velocity.z());
}

V3 RigidbodyBox2D::angularFactor() const
{
    return V3(0, 0, 1);
}

void RigidbodyBox2D::setAngularFactor(const V3& /*factor*/)
{
}

void RigidbodyBox2D::setLinearVelocity(const V3& velocity)
{
    b2Body_SetLinearVelocity(_stub->body(), {velocity.x(), velocity.y()});
}

V3 RigidbodyBox2D::linearFactor() const
{
    return V3(1, 1, 0);
}

void RigidbodyBox2D::setLinearFactor(const V3& factor)
{
}

void RigidbodyBox2D::applyCentralImpulse(const V3& impulse)
{
    b2Body_ApplyLinearImpulseToCenter(_stub->body(), {impulse.x(), impulse.y()}, true);
}

float RigidbodyBox2D::friction() const
{
    FATAL("Unimplemented");
    return 0;
}

void RigidbodyBox2D::setFriction(float friction)
{
    FATAL("Unimplemented");
}

float RigidbodyBox2D::gravityScale() const
{
    return b2Body_GetGravityScale(_stub->body());
}

void RigidbodyBox2D::setGravityScale(float scale)
{
    b2Body_SetGravityScale(_stub->body(), scale);
}

bool RigidbodyBox2D::active() const
{
    return b2Body_IsAwake(_stub->_body);
}

void RigidbodyBox2D::setActive(const bool active)
{
    b2Body_SetAwake(_stub->_body, active);
}

float RigidbodyBox2D::mass() const
{
    return b2Body_GetMass(_stub->body());
}

void RigidbodyBox2D::setMass(const float mass)
{
    b2MassData massData = b2Body_GetMassData(_stub->body());
    massData.mass = mass;
    b2Body_SetMassData(_stub->body(), massData);
}

void RigidbodyBox2D::applyTorque(float torque, bool wake)
{
    b2Body_ApplyTorque(_stub->_body, torque, wake);
}

void RigidbodyBox2D::applyForce(const V2& force, const V2& point, bool wake)
{
    b2Body_ApplyForce(_stub->_body, {force.x(), force.y()}, {point.x(), point.y()}, wake);
}

void RigidbodyBox2D::applyCentralForce(const V3& force)
{
    b2Body_ApplyForceToCenter(_stub->_body, {force.x(), force.y()}, true);
}

void RigidbodyBox2D::applyLinearImpulse(const V2& impulse, const V2& point, bool wake)
{
    b2Body_ApplyLinearImpulse(_stub->_body, {impulse.x(), impulse.y()}, {point.x(), point.y()}, true);
}

void RigidbodyBox2D::applyAngularImpulse(float impulse, bool wake)
{
    b2Body_ApplyAngularImpulse(_stub->_body, impulse, wake);
}

void RigidbodyBox2D::setTransform(const V2& position, float angle)
{
    b2Body_SetTransform(_stub->_body, {position.x(), position.y()}, b2MakeRot(angle));
}

sp<Future> RigidbodyBox2D::applyLinearVelocity(const sp<Vec2>& velocity)
{
    sp<Future> future = sp<Future>::make();
    sp<ManualLinearVelocity> task = sp<ManualLinearVelocity>::make(_stub, velocity, future);
    Ark::instance().applicationContext()->addPreComposeRunnable(std::move(task), future->isDoneOrCanceled());
    return future;
}

sp<Future> RigidbodyBox2D::applyRotate(const sp<Numeric>& rotate)
{
    sp<Future> future = sp<Future>::make();
    sp<Runnable> task = sp<ManualApplyRotate>::make(_stub, rotate, future);
    Ark::instance().applicationContext()->addPreComposeRunnable(std::move(task), future->isDoneOrCanceled());
    return future;
}

Rigidbody RigidbodyBox2D::makeShadow() const
{
    return {{_rigidbody_stub, nullptr}, true};
}

RigidbodyBox2D::Stub::Stub(const ColliderBox2D& world, b2BodyId body)
    : _world(world), _body(body)
{
}

RigidbodyBox2D::Stub::~Stub()
{
    if(b2Body_IsValid(_body))
        dispose();
}

void RigidbodyBox2D::Stub::dispose()
{
    CHECK(b2Body_IsValid(_body), "Body has been disposed already");

    b2Body_SetUserData(_body, nullptr);
    _discarded.reset(true);

    BodyDisposer disposer(_world, _body);

    _body = b2_nullBodyId;
}

b2BodyId RigidbodyBox2D::Stub::body()
{
    CHECK(b2Body_IsValid(_body), "Body has been disposed");
    return _body;
}

}
