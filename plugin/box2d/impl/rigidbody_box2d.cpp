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
#include "graphics/components/shape.h"

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
        DCHECK(_stub->_body, "Body has been disposed already");
        return _stub->_body->GetAngle();
    }

    bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    sp<RigidbodyBox2D::Stub> _stub;
};

class _RigidBodyPosition : public Vec3 {
public:
    _RigidBodyPosition(const sp<RigidbodyBox2D::Stub>& stub, const sp<Vec3>& delegate)
        : _stub(stub)/*, _delegate(delegate)*/ {
    }

    virtual V3 val() override {
        DCHECK(_stub->_body, "Body has been disposed");
        return V3(_stub->_body->GetPosition().x, _stub->_body->GetPosition().y, 0);
    }

    virtual bool update(uint64_t /*timestamp*/) override {
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

    virtual V3 val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        float x = _stub->_world.toPixelX(_stub->_body->GetPosition().x);
        float y = _stub->_world.toPixelY(_stub->_body->GetPosition().y);
        return V3(x, y, 0);
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

private:
    sp<RigidbodyBox2D::Stub> _stub;
};

class ManualRotation : public Numeric {
public:
    ManualRotation(sp<RigidbodyBox2D::Stub> stub, sp<Numeric> delegate)
        : _stub(std::move(stub)), _delegate(std::move(delegate)) {
    }

    virtual float val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        float rotation = _delegate->val();
        _stub->_body->SetTransform(_stub->_body->GetWorldCenter(), rotation);
        return rotation;
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

private:
    sp<RigidbodyBox2D::Stub> _stub;
    sp<Numeric> _delegate;
};

class ManualLinearVelocity : public Runnable {
public:
    ManualLinearVelocity(const sp<RigidbodyBox2D::Stub>& body, const sp<Vec2>& velocity, const sp<Future>& future)
        : _body(body), _velocity(velocity), _future(future) {
    }

    virtual void run() override {
        const V2 v = _velocity->val();
        if(_body->_body) {
            _body->_body->SetLinearVelocity(b2Vec2(v.x(), v.y()));
        } else if(_future) {
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

    virtual void run() override {
        if(_body->_body) {
            float rotation = _rotate->val();
            _body->_body->SetTransform(_body->_body->GetWorldCenter(), rotation);
        } else if(_future) {
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
    BodyDisposer(const ColliderBox2D& world, b2Body* body)
        : _world(world), _body(body) {
    }
    ~BodyDisposer() {
        DCHECK(_body, "Body has been disposed already");
        DCHECK(!_world.world().IsLocked(), "Cannot destroy body in the middle of a time step");
        _world.world().DestroyBody(_body);
    }

private:
    ColliderBox2D _world;
    b2Body* _body;
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
    : _rigidbody_stub(sp<Rigidbody::Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, sp<ark::Shape>::make(0, sp<Vec3>::make<Vec3::Const>(size)), sp<_RigidBodyPosition>::make(stub, position), sp<Vec4>::make<RotationAxisTheta>(sp<Vec3>::make<Vec3::Const>(V3(constants::AXIS_Z)), sp<Numeric>::make<_RigidBodyRotation>(stub)), std::move(collisionFilter))),
      _stub(stub)
{
    _stub->_body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
}

const sp<Rigidbody::Stub>& RigidbodyBox2D::rigidbodyStub() const
{
    return _rigidbody_stub;
}

b2Body* RigidbodyBox2D::body() const
{
    return _stub->body();
}

float RigidbodyBox2D::angle()
{
    return _stub->body()->GetAngle();
}

void RigidbodyBox2D::setAngle(float rad)
{
    _stub->body()->SetTransform(_stub->_body->GetWorldCenter(), rad);
}

float RigidbodyBox2D::angularVelocity()
{
    return _stub->body()->GetAngularVelocity();
}

void RigidbodyBox2D::setAngularVelocity(float omega)
{
    _stub->body()->SetAngularVelocity(omega);
}

V3 RigidbodyBox2D::linearVelocity() const
{
    const b2Vec2 velocity = _stub->body()->GetLinearVelocity();
    return {velocity.x, velocity.y, 0};
}

void RigidbodyBox2D::setLinearVelocity(const V3& velocity)
{
    _stub->body()->SetLinearVelocity(b2Vec2(velocity.x(), velocity.y()));
}

void RigidbodyBox2D::applyCentralImpulse(const V3& impulse)
{
    _stub->body()->ApplyLinearImpulseToCenter(b2Vec2(impulse.x(), impulse.y()), true);
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
    return _stub->body()->GetGravityScale();
}

void RigidbodyBox2D::setGravityScale(float scale)
{
    _stub->body()->SetGravityScale(scale);
}

bool RigidbodyBox2D::active() const
{
    return _stub->body()->IsEnabled();
}

void RigidbodyBox2D::setActive(bool active)
{
    CHECK(!_stub->_world.world().IsLocked(), "Cannot set active in the middle of a time step");
    _stub->body()->SetEnabled(active);
}

bool RigidbodyBox2D::awake()
{
    return _stub->body()->IsAwake();
}

void RigidbodyBox2D::setAwake(bool awake)
{
    _stub->body()->SetAwake(awake);
}

float RigidbodyBox2D::x() const
{
    return _stub->body()->GetPosition().x;
}

float RigidbodyBox2D::y() const
{
    return _stub->body()->GetPosition().y;
}

float RigidbodyBox2D::mass() const
{
    return _stub->body()->GetMass();
}

void RigidbodyBox2D::applyTorque(float torque, bool wake)
{
    _stub->body()->ApplyTorque(torque, wake);
}

void RigidbodyBox2D::applyForce(const V2& force, const V2& point, bool wake)
{
    _stub->body()->ApplyForce(b2Vec2(force.x(), force.y()), b2Vec2(point.x(), point.y()), wake);
}

void RigidbodyBox2D::applyForceToCenter(const V2& force, bool wake)
{
    _stub->body()->ApplyForceToCenter(b2Vec2(force.x(), force.y()), wake);
}

void RigidbodyBox2D::applyLinearImpulse(const V2& impulse, const V2& point, bool wake)
{
    _stub->body()->ApplyLinearImpulse(b2Vec2(impulse.x(), impulse.y()), b2Vec2(point.x(), point.y()), wake);
}

void RigidbodyBox2D::applyAngularImpulse(float impulse, bool wake)
{
    _stub->body()->ApplyAngularImpulse(impulse, wake);
}

void RigidbodyBox2D::setTransform(const V2& position, float angle)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set transform in the middle of a time step");
    _stub->body()->SetTransform(b2Vec2(position.x(), position.y()), angle);
}

sp<Future> RigidbodyBox2D::applyLinearVelocity(const sp<Vec2>& velocity)
{
    const sp<Future> future = sp<Future>::make();
    const sp<ManualLinearVelocity> task = sp<ManualLinearVelocity>::make(_stub, velocity, future);
    Ark::instance().applicationContext()->addPreComposeRunnable(task, future->isCanceled());
    return future;
}

sp<Future> RigidbodyBox2D::applyRotate(const sp<Numeric>& rotate)
{
    const sp<Future> future = sp<Future>::make();
    const sp<Runnable> task = sp<ManualApplyRotate>::make(_stub, rotate, future);
    Ark::instance().applicationContext()->addPreComposeRunnable(task, future->isCanceled());
    return future;
}

Rigidbody RigidbodyBox2D::makeShadow() const
{
    return {{_rigidbody_stub, nullptr}, true};
}

RigidbodyBox2D::Stub::Stub(const ColliderBox2D& world, b2Body* body)
    : _world(world), _body(body)
{
}

RigidbodyBox2D::Stub::~Stub()
{
    if(_body)
        dispose();
}

void RigidbodyBox2D::Stub::dispose()
{
    CHECK(_body, "Body has been disposed already");

    _body->GetUserData().pointer = 0;
    _discarded.reset(true);

    if(_world.world().IsLocked())
        Ark::instance().applicationContext()->deferUnref(sp<BodyDisposer>::make(_world, _body));
    else
        BodyDisposer disposer(_world, _body);

    _body = nullptr;
}

b2Body* RigidbodyBox2D::Stub::body()
{
    CHECK(_body, "Body has been disposed");
    return _body;
}

}
