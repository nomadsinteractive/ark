#include "box2d/impl/body.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/future.h"
#include "core/inf/variable.h"
#include "core/types/weak_ptr.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/rotate.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

namespace {

class _RigidBodyRotation : public Numeric {
public:
    _RigidBodyRotation(const sp<Body::Stub>& stub, const sp<Numeric>& delegate)
        : _stub(stub), _delegate(delegate) {
    }

    virtual float val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        return _stub->_body->GetAngle();
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    sp<Body::Stub> _stub;
    sp<Numeric> _delegate;
};

class _RigidBodyPosition : public Vec {
public:
    _RigidBodyPosition(const sp<Body::Stub>& stub, const sp<Vec>& delegate)
        : _stub(stub)/*, _delegate(delegate)*/ {
    }

    virtual V val() override {
        DCHECK(_stub->_body, "Body has been disposed");
        return V(_stub->_body->GetPosition().x, _stub->_body->GetPosition().y);
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    sp<Body::Stub> _stub;
    sp<Vec> _delegate;

};

class RenderObjectPosition : public Vec3 {
public:
    RenderObjectPosition(const sp<Body::Stub>& stub)
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
    sp<Body::Stub> _stub;
};

class ManualRotation : public Numeric {
public:
    ManualRotation(const sp<Body::Stub>& stub, const sp<Numeric>& delegate)
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
    sp<Body::Stub> _stub;
    sp<Numeric> _delegate;
};

class ManualLinearVelocity : public Runnable {
public:
    ManualLinearVelocity(const sp<Body::Stub>& body, const sp<Vec2>& velocity, const sp<Future>& future)
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
    sp<Body::Stub> _body;
    sp<Vec2> _velocity;
    sp<Future> _future;
};

class ManualApplyRotate : public Runnable {
public:
    ManualApplyRotate(const sp<Body::Stub>& body, const sp<Numeric>& rotate, const sp<Future>& future)
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
    sp<Body::Stub> _body;
    sp<Numeric> _rotate;
    sp<Future> _future;
};

class BodyDisposer {
public:
    BodyDisposer(const World& world, b2Body* body)
        : _world(world), _body(body) {
    }
    ~BodyDisposer() {
        DCHECK(_body, "Body has been disposed already");
        DCHECK(!_world.world().IsLocked(), "Cannot destroy body in the middle of a time step");
        _world.world().DestroyBody(_body);
    }

private:
    World _world;
    b2Body* _body;
};

}

Body::Body(const World& world, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotate, const sp<Shape>& shape, float density, float friction, bool isSensor)
    : Body(world, type, position, size, rotate, BodyCreateInfo(shape, density, friction, isSensor))
{
}

Body::Body(const World& world, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotate, const BodyCreateInfo& createInfo)
    : Body(sp<Stub>::make(world, world.createBody(type, position->val(), size, createInfo)), type, position, size, rotate)
{
}

Body::Body(const sp<Stub>& stub, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotation)
    : RigidBody(stub->_id, type,
                sp<_RigidBodyPosition>::make(stub, position),
                size,
                sp<Rotate>::make(sp<_RigidBodyRotation>::make(stub, rotation)), stub->_disposed), _stub(stub)
{
    _stub->_callback = callback();
    _stub->_body->SetUserData(new Shadow(stub, RigidBody::stub()));
}

Body::Body(const sp<Body::Stub>& stub, const sp<RigidBody::Stub>& rigidbody)
    : RigidBody(rigidbody), _stub(stub)
{
}

void Body::bind(const sp<RenderObject>& renderObject)
{
    renderObject->setPosition(sp<RenderObjectPosition>::make(_stub));
    renderObject->setTransform(transform());
    if(type() & Collider::BODY_FLAG_MANUAL_ROTATION)
    {
        const sp<Numeric> r = transform()->rotate() ? transform()->rotate()->value()->delegate().cast<_RigidBodyRotation>()->_delegate : sp<Numeric>::null();
        transform()->rotate()->setRotation(sp<ManualRotation>::make(_stub, r));
    }
}

void Body::dispose()
{
    _stub->dispose();
}

sp<Body> Body::obtain(const Shadow* shadow)
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
        const sp<Vec2> p = sp<Vec2::Const>::make(V(position.x, position.y));
        const sp<Rotate> rotate = sp<Rotate>::make(sp<Numeric::Const>::make(rotation));
        rigidBodyStub = sp<RigidBody::Stub>::make(s._id, bodyType, p, nullptr, rotate, s._disposed, nullptr, shadow->_tag);
    }
    return sp<Body>::make(shadow->_body.ensure(), rigidBodyStub);
}

b2Body* Body::body() const
{

    return _stub->body();
}

float Body::angle()
{
    return _stub->body()->GetAngle();
}

void Body::setAngle(float rad)
{
    _stub->body()->SetTransform(_stub->_body->GetWorldCenter(), rad);
}

float Body::angularVelocity()
{
    return _stub->body()->GetAngularVelocity();
}

void Body::setAngularVelocity(float omega)
{
    _stub->body()->SetAngularVelocity(omega);
}

V2 Body::linearVelocity() const
{
    const b2Vec2 velocity = _stub->body()->GetLinearVelocity();
    return V2(velocity.x, velocity.y);
}

void Body::setLinearVelocity(const V2& velocity)
{
    _stub->body()->SetLinearVelocity(b2Vec2(velocity.x(), velocity.y()));
}

float Body::gravityScale() const
{
    return _stub->body()->GetGravityScale();
}

void Body::setGravityScale(float scale)
{
    _stub->body()->SetGravityScale(scale);
}

bool Body::active()
{
    return _stub->body()->IsActive();
}

void Body::setActive(bool active)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set active in the middle of a time step");
    _stub->body()->SetActive(active);
}

bool Body::awake()
{
    return _stub->body()->IsAwake();
}

void Body::setAwake(bool awake)
{
    _stub->body()->SetAwake(awake);
}

float Body::x() const
{
    return _stub->body()->GetPosition().x;
}

float Body::y() const
{
    return _stub->body()->GetPosition().y;
}

float Body::mass() const
{
    return _stub->body()->GetMass();
}

void Body::applyTorque(float torque, bool wake)
{
    _stub->body()->ApplyTorque(torque, wake);
}

void Body::applyForce(const V2& force, const V2& point, bool wake)
{
    _stub->body()->ApplyForce(b2Vec2(force.x(), force.y()), b2Vec2(point.x(), point.y()), wake);
}

void Body::applyForceToCenter(const V2& force, bool wake)
{
    _stub->body()->ApplyForceToCenter(b2Vec2(force.x(), force.y()), wake);
}

void Body::applyLinearImpulse(const V2& impulse, const V2& point, bool wake)
{
    _stub->body()->ApplyLinearImpulse(b2Vec2(impulse.x(), impulse.y()), b2Vec2(point.x(), point.y()), wake);
}

void Body::applyAngularImpulse(float impulse, bool wake)
{
    _stub->body()->ApplyAngularImpulse(impulse, wake);
}

void Body::setTransform(const V2& position, float angle)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set transform in the middle of a time step");
    _stub->body()->SetTransform(b2Vec2(position.x(), position.y()), angle);
}

sp<Future> Body::applyLinearVelocity(const sp<Vec2>& velocity)
{
    const sp<Future> future = sp<Future>::make();
    const sp<ManualLinearVelocity> task = sp<ManualLinearVelocity>::make(_stub, velocity, future);
    Ark::instance().applicationContext()->addPreRenderTask(task, future->cancelled());
    return future;
}

sp<Future> Body::applyRotate(const sp<Numeric>& rotate)
{
    const sp<Future> future = sp<Future>::make();
    const sp<Runnable> task = sp<ManualApplyRotate>::make(_stub, rotate, future);
    Ark::instance().applicationContext()->addPreRenderTask(task, future->cancelled());
    return future;
}

Body::Stub::Stub(const World& world, b2Body* body)
    : _id(world.genRigidBodyId()), _world(world), _body(body)
{
}

Body::Stub::~Stub()
{
    if(_body)
        dispose();
}

void Body::Stub::dispose()
{
    DCHECK(_body, "Body has been disposed already");
    LOGD("id = %d", _id);

    delete reinterpret_cast<Body::Shadow*>(_body->GetUserData());
    _body->SetUserData(nullptr);
    _disposed->dispose();

    if(_world.world().IsLocked())
        Ark::instance().applicationContext()->deferUnref(sp<BodyDisposer>::make(_world, _body));
    else
        BodyDisposer disposer(_world, _body);

    _body = nullptr;
    _callback = nullptr;
}

b2Body* Body::Stub::body()
{
    DCHECK(_body, "Body has been disposed");
    return _body;
}

Body::Shadow::Shadow(const sp<Body::Stub>& body, const sp<RigidBody::Stub>& rigidBody)
    : _body(body), _rigid_body(rigidBody), _tag(rigidBody->_tag)
{
}

}
}
}
