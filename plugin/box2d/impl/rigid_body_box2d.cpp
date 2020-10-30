#include "box2d/impl/rigid_body_box2d.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/future.h"
#include "core/inf/variable.h"
#include "core/types/weak_ptr.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_type.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

namespace {

class _RigidBodyRotation : public Numeric {
public:
    _RigidBodyRotation(const sp<RigidBodyBox2D::Stub>& stub, const sp<Numeric>& delegate)
        : _stub(stub), _delegate(delegate) {
    }

    virtual float val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        return _stub->_body->GetAngle();
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    sp<RigidBodyBox2D::Stub> _stub;
    sp<Numeric> _delegate;
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
    ManualRotation(const sp<RigidBodyBox2D::Stub>& stub, const sp<Numeric>& delegate)
        : _stub(stub), _delegate(delegate) {
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

RigidBodyBox2D::RigidBodyBox2D(const ColliderBox2D& world, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Numeric>& rotate, const sp<Shape>& shape, float density, float friction)
    : RigidBodyBox2D(world, type, position, size, rotate, BodyCreateInfo(shape, density, friction, type & Collider::BODY_TYPE_SENSOR))
{
}

RigidBodyBox2D::RigidBodyBox2D(const ColliderBox2D& world, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Numeric>& rotate, const BodyCreateInfo& createInfo)
    : RigidBodyBox2D(sp<Stub>::make(world, world.createBody(type, position->val(), size, createInfo)), type, position, size, rotate)
{
}

RigidBodyBox2D::RigidBodyBox2D(const sp<Stub>& stub, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Numeric>& rotation)
    : RigidBody(stub->_id, type, sp<_RigidBodyPosition>::make(stub, position), size,
                sp<Rotation>::make(sp<_RigidBodyRotation>::make(stub, rotation)), Box(), stub->_disposed), _stub(stub)
{
    _stub->_callback = callback();
    _stub->_body->SetUserData(new Shadow(stub, RigidBody::stub()));
}

RigidBodyBox2D::RigidBodyBox2D(const sp<RigidBodyBox2D::Stub>& stub, const sp<RigidBody::Stub>& rigidbody)
    : RigidBody(rigidbody), _stub(stub)
{
}

void RigidBodyBox2D::bind(const sp<RenderObject>& renderObject)
{
    renderObject->setPosition(sp<RenderObjectPosition>::make(_stub));
    renderObject->setTransform(transform());
    if(type() & Collider::BODY_FLAG_MANUAL_ROTATION)
    {
        const sp<Numeric> r = transform()->rotation() ? transform()->rotation()->theta().cast<_RigidBodyRotation>()->_delegate : sp<Numeric>::null();
        transform()->rotation()->setRotation(sp<ManualRotation>::make(_stub, r), nullptr);
    }
}

void RigidBodyBox2D::dispose()
{
    _stub->dispose();
}

sp<RigidBodyBox2D> RigidBodyBox2D::obtain(const Shadow* shadow)
{
    const sp<Stub> bodyStub = shadow->_body.ensure();
    sp<RigidBody::Stub> rigidBodyStub = shadow->_rigid_body.lock();
    if(!rigidBodyStub)
    {
        const Stub& s = bodyStub;
        Collider::BodyType bodyType = (s._body->GetType() == b2_staticBody) ?
                    Collider::BODY_TYPE_STATIC :
                    (s._body->GetType() == b2_kinematicBody ? Collider::BODY_TYPE_KINEMATIC : Collider::BODY_TYPE_DYNAMIC);
        const b2Vec2& position = s._body->GetPosition();
        float rotation = s._body->GetTransform().q.GetAngle();
        const sp<Vec3> p = sp<Vec3::Const>::make(V3(position.x, position.y, 0));
        const sp<Rotation> rotate = sp<Rotation>::make(sp<Numeric::Const>::make(rotation));
        rigidBodyStub = sp<RigidBody::Stub>::make(s._id, bodyType, p, nullptr, rotate, Box(), s._disposed);
        rigidBodyStub->_tag = shadow->_tag;
    }
    return sp<RigidBodyBox2D>::make(shadow->_body.ensure(), rigidBodyStub);
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
    return _stub->body()->IsActive();
}

void RigidBodyBox2D::setActive(bool active)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set active in the middle of a time step");
    _stub->body()->SetActive(active);
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
    Ark::instance().applicationContext()->addPreRenderTask(task, future->cancelled());
    return future;
}

sp<Future> RigidBodyBox2D::applyRotate(const sp<Numeric>& rotate)
{
    const sp<Future> future = sp<Future>::make();
    const sp<Runnable> task = sp<ManualApplyRotate>::make(_stub, rotate, future);
    Ark::instance().applicationContext()->addPreRenderTask(task, future->cancelled());
    return future;
}

RigidBodyBox2D::Stub::Stub(const ColliderBox2D& world, b2Body* body)
    : _id(world.genRigidBodyId()), _world(world), _body(body)
{
}

RigidBodyBox2D::Stub::~Stub()
{
    if(_body)
        dispose();
}

void RigidBodyBox2D::Stub::dispose()
{
    DCHECK(_body, "Body has been disposed already");
    LOGD("id = %d", _id);

    delete reinterpret_cast<RigidBodyBox2D::Shadow*>(_body->GetUserData());
    _body->SetUserData(nullptr);
    _disposed->dispose();

    if(_world.world().IsLocked())
        Ark::instance().applicationContext()->deferUnref(sp<BodyDisposer>::make(_world, _body));
    else
        BodyDisposer disposer(_world, _body);

    _body = nullptr;
    _callback = nullptr;
}

b2Body* RigidBodyBox2D::Stub::body()
{
    DCHECK(_body, "Body has been disposed");
    return _body;
}

RigidBodyBox2D::Shadow::Shadow(const sp<RigidBodyBox2D::Stub>& body, const sp<RigidBody::Stub>& rigidBody)
    : _body(body), _rigid_body(rigidBody), _tag(rigidBody->_tag)
{
}

}
}
}
