#include "box2d/impl/rigid_body_box2d.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/future.h"
#include "core/inf/variable.h"
#include "core/types/weak_ptr.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_type.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "app/base/application_context.h"
#include "app/traits/shape.h"

#include "box2d/impl/collider_box2d.h"

namespace ark::plugin::box2d {

namespace {

class _RigidBodyRotation final : public Numeric {
public:
    _RigidBodyRotation(const sp<RigidBodyBox2D::Stub>& stub)
        : _stub(stub) {
    }

    float val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        return _stub->_body->GetAngle();
    }

    bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    sp<RigidBodyBox2D::Stub> _stub;
};

class _RigidBodyPosition : public Vec3 {
public:
    _RigidBodyPosition(const sp<RigidBodyBox2D::Stub>& stub, const sp<Vec3>& delegate)
        : _stub(stub)/*, _delegate(delegate)*/ {
    }

    virtual V3 val() override {
        DCHECK(_stub->_body, "Body has been disposed");
        return V3(_stub->_body->GetPosition().x, _stub->_body->GetPosition().y, 0);
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    sp<RigidBodyBox2D::Stub> _stub;
    sp<Vec3> _delegate;

};

class RenderObjectPosition : public Vec3 {
public:
    RenderObjectPosition(const sp<RigidBodyBox2D::Stub>& stub)
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
    sp<RigidBodyBox2D::Stub> _stub;
};

class ManualRotation : public Numeric {
public:
    ManualRotation(sp<RigidBodyBox2D::Stub> stub, sp<Numeric> delegate)
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
    sp<RigidBodyBox2D::Stub> _stub;
    sp<Numeric> _delegate;
};

class ManualLinearVelocity : public Runnable {
public:
    ManualLinearVelocity(const sp<RigidBodyBox2D::Stub>& body, const sp<Vec2>& velocity, const sp<Future>& future)
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
    sp<RigidBodyBox2D::Stub> _body;
    sp<Vec2> _velocity;
    sp<Future> _future;
};

class ManualApplyRotate : public Runnable {
public:
    ManualApplyRotate(const sp<RigidBodyBox2D::Stub>& body, const sp<Numeric>& rotate, const sp<Future>& future)
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
    sp<RigidBodyBox2D::Stub> _body;
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

RigidBodyBox2D::RigidBodyBox2D(const ColliderBox2D& world, Collider::BodyType type, const sp<Vec3>& position, const V3& size, const SafeVar<Numeric>& rotate, const sp<Shape>& shape, float density, float friction)
    : RigidBodyBox2D(world, type, position, size, rotate, BodyCreateInfo(shape, density, friction, type & Collider::BODY_TYPE_SENSOR))
{
}

RigidBodyBox2D::RigidBodyBox2D(const ColliderBox2D& world, Collider::BodyType type, const sp<Vec3>& position, const V3& size, const SafeVar<Numeric>& rotate, const BodyCreateInfo& createInfo)
    : RigidBodyBox2D(sp<Stub>::make(world, world.createBody(type, position->val(), size, createInfo)), type, position, size, rotate)
{
}

//TODO: Manual rotation
RigidBodyBox2D::RigidBodyBox2D(const sp<Stub>& stub, Collider::BodyType type, const sp<Vec3>& position, const V3& size, const SafeVar<Numeric>& rotation)
    : RigidBody(type, sp<ark::Shape>::make(0, sp<Vec3>::make<Vec3::Const>(size)), sp<_RigidBodyPosition>::make(stub, position),
                sp<Rotation>::make(sp<Numeric>::make<_RigidBodyRotation>(stub)), Box(), nullptr), _stub(stub)
{
    _stub->_body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
}

void RigidBodyBox2D::discard()
{
    _stub->dispose();
}

b2Body* RigidBodyBox2D::body() const
{

    return _stub->body();
}

float RigidBodyBox2D::angle()
{
    return _stub->body()->GetAngle();
}

void RigidBodyBox2D::setAngle(float rad)
{
    _stub->body()->SetTransform(_stub->_body->GetWorldCenter(), rad);
}

float RigidBodyBox2D::angularVelocity()
{
    return _stub->body()->GetAngularVelocity();
}

void RigidBodyBox2D::setAngularVelocity(float omega)
{
    _stub->body()->SetAngularVelocity(omega);
}

V2 RigidBodyBox2D::linearVelocity() const
{
    const b2Vec2 velocity = _stub->body()->GetLinearVelocity();
    return V2(velocity.x, velocity.y);
}

void RigidBodyBox2D::setLinearVelocity(const V2& velocity)
{
    _stub->body()->SetLinearVelocity(b2Vec2(velocity.x(), velocity.y()));
}

float RigidBodyBox2D::gravityScale() const
{
    return _stub->body()->GetGravityScale();
}

void RigidBodyBox2D::setGravityScale(float scale)
{
    _stub->body()->SetGravityScale(scale);
}

bool RigidBodyBox2D::active()
{
    return _stub->body()->IsEnabled();
}

void RigidBodyBox2D::setActive(bool active)
{
    CHECK(!_stub->_world.world().IsLocked(), "Cannot set active in the middle of a time step");
    _stub->body()->SetEnabled(active);
}

bool RigidBodyBox2D::awake()
{
    return _stub->body()->IsAwake();
}

void RigidBodyBox2D::setAwake(bool awake)
{
    _stub->body()->SetAwake(awake);
}

float RigidBodyBox2D::x() const
{
    return _stub->body()->GetPosition().x;
}

float RigidBodyBox2D::y() const
{
    return _stub->body()->GetPosition().y;
}

float RigidBodyBox2D::mass() const
{
    return _stub->body()->GetMass();
}

void RigidBodyBox2D::applyTorque(float torque, bool wake)
{
    _stub->body()->ApplyTorque(torque, wake);
}

void RigidBodyBox2D::applyForce(const V2& force, const V2& point, bool wake)
{
    _stub->body()->ApplyForce(b2Vec2(force.x(), force.y()), b2Vec2(point.x(), point.y()), wake);
}

void RigidBodyBox2D::applyForceToCenter(const V2& force, bool wake)
{
    _stub->body()->ApplyForceToCenter(b2Vec2(force.x(), force.y()), wake);
}

void RigidBodyBox2D::applyLinearImpulse(const V2& impulse, const V2& point, bool wake)
{
    _stub->body()->ApplyLinearImpulse(b2Vec2(impulse.x(), impulse.y()), b2Vec2(point.x(), point.y()), wake);
}

void RigidBodyBox2D::applyAngularImpulse(float impulse, bool wake)
{
    _stub->body()->ApplyAngularImpulse(impulse, wake);
}

void RigidBodyBox2D::setTransform(const V2& position, float angle)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set transform in the middle of a time step");
    _stub->body()->SetTransform(b2Vec2(position.x(), position.y()), angle);
}

sp<Future> RigidBodyBox2D::applyLinearVelocity(const sp<Vec2>& velocity)
{
    const sp<Future> future = sp<Future>::make();
    const sp<ManualLinearVelocity> task = sp<ManualLinearVelocity>::make(_stub, velocity, future);
    Ark::instance().applicationContext()->addPreRenderTask(task, future->canceled());
    return future;
}

sp<Future> RigidBodyBox2D::applyRotate(const sp<Numeric>& rotate)
{
    const sp<Future> future = sp<Future>::make();
    const sp<Runnable> task = sp<ManualApplyRotate>::make(_stub, rotate, future);
    Ark::instance().applicationContext()->addPreRenderTask(task, future->canceled());
    return future;
}

RigidBodyBox2D::Stub::Stub(const ColliderBox2D& world, b2Body* body)
    : _world(world), _body(body)
{
}

RigidBodyBox2D::Stub::~Stub()
{
    if(_body)
        dispose();
}

void RigidBodyBox2D::Stub::dispose()
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

b2Body* RigidBodyBox2D::Stub::body()
{
    CHECK(_body, "Body has been disposed");
    return _body;
}

}
