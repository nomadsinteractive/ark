#include "box2d/impl/body.h"

#include "core/base/bean_factory.h"
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
#include "graphics/util/vec2_util.h"

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

    sp<Body::Stub> _stub;
    sp<Vec> _delegate;
};

class RenderObjectPosition : public Vec {
public:
    RenderObjectPosition(const sp<Body::Stub>& stub)
        : _stub(stub) {
    }

    virtual V val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        float x = _stub->_world.toPixelX(_stub->_body->GetPosition().x);
        float y = _stub->_world.toPixelY(_stub->_body->GetPosition().y);
        return V(x, y);
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
        float rotation = _delegate->val();
        _stub->_body->SetTransform(_stub->_body->GetWorldCenter(), rotation);
        return rotation;
    }

private:
    sp<Body::Stub> _stub;
    sp<Numeric> _delegate;
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
                sp<Rotate>::make(sp<_RigidBodyRotation>::make(stub, rotation))), _stub(stub)
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
        transform()->rotate()->setRadians(sp<ManualRotation>::make(_stub, r));
    }
}

void Body::dispose()
{
    _stub->dispose();
}

sp<Body> Body::obtain(const Shadow* shadow, ObjectPool& objectPool)
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
        const sp<Vec> p = objectPool.obtain<Vec::Const>(V(position.x, position.y));
        const sp<Rotate> rotate = objectPool.obtain<Rotate>(objectPool.obtain<Numeric::Const>(rotation));
        rigidBodyStub = objectPool.obtain<RigidBody::Stub>(s._id, bodyType, p, nullptr, rotate, nullptr, shadow->_tag);
    }
    return objectPool.obtain<Body>(shadow->_body.ensure(), rigidBodyStub);
}

b2Body* Body::body() const
{
    return _stub->_body;
}

float Body::angle()
{
    return _stub->_body->GetAngle();
}

void Body::setAngle(float rad)
{
    _stub->_body->SetTransform(_stub->_body->GetWorldCenter(), rad);
}

float Body::angularVelocity()
{
    return _stub->_body->GetAngularVelocity();
}

void Body::setAngularVelocity(float omega)
{
    _stub->_body->SetAngularVelocity(omega);
}

V2 Body::linearVelocity() const
{
    const b2Vec2 velocity = _stub->_body->GetLinearVelocity();
    return V2(velocity.x, velocity.y);
}

void Body::setLinearVelocity(const V2& velocity)
{
    _stub->_body->SetLinearVelocity(b2Vec2(velocity.x(), velocity.y()));
}

float Body::gravityScale() const
{
    return _stub->_body->GetGravityScale();
}

void Body::setGravityScale(float scale)
{
    _stub->_body->SetGravityScale(scale);
}

bool Body::active()
{
    return _stub->_body->IsActive();
}

void Body::setActive(bool active)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set active in the middle of a time step");
    _stub->_body->SetActive(active);
}

bool Body::awake()
{
    return _stub->_body->IsAwake();
}

void Body::setAwake(bool awake)
{
    _stub->_body->SetAwake(awake);
}

float Body::x() const
{
    return _stub->_body->GetPosition().x;
}

float Body::y() const
{
    return _stub->_body->GetPosition().y;
}

float Body::mass() const
{
    return _stub->_body->GetMass();
}

void Body::applyTorque(float torque, bool wake)
{
    _stub->_body->ApplyTorque(torque, wake);
}

void Body::applyForce(const V2& force, const V2& point, bool wake)
{
    _stub->_body->ApplyForce(b2Vec2(force.x(), force.y()), b2Vec2(point.x(), point.y()), wake);
}

void Body::applyForceToCenter(const V2& force, bool wake)
{
    _stub->_body->ApplyForceToCenter(b2Vec2(force.x(), force.y()), wake);
}

void Body::applyLinearImpulse(const V2& impulse, const V2& point, bool wake)
{
    _stub->_body->ApplyLinearImpulse(b2Vec2(impulse.x(), impulse.y()), b2Vec2(point.x(), point.y()), wake);
}

void Body::applyAngularImpulse(float impulse, bool wake)
{
    _stub->_body->ApplyAngularImpulse(impulse, wake);
}

void Body::setTransform(const V2& position, float angle)
{
    DCHECK(!_stub->_world.world().IsLocked(), "Cannot set transform in the middle of a time step");
    _stub->_body->SetTransform(b2Vec2(position.x(), position.y()), angle);
}

Body::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest)
    : _world(factory.ensureBuilder<Object>(manifest, "world")),
      _shape(factory.ensureBuilder<Shape>(manifest, "shape")),
      _position(factory.ensureBuilder<Vec>(manifest, Constants::Attributes::POSITION)),
      _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _density(Documents::getAttribute<float>(manifest, "density", 1.0f)),
      _friction(Documents::getAttribute<float>(manifest, "friction", 0.2f))
{
    DCHECK(_world, "Illegal world object");
}

sp<Body> Body::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    const sp<World> world = BeanUtils::as<World>(_world, args);
    const sp<Shape> shape = _shape->build(args);
    const sp<Vec> position = _position->build(args);
    return sp<Body>::make(world, Collider::BODY_TYPE_DYNAMIC, position, _size->build(args), nullptr, shape, _density, _friction);
}

Body::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& parent, const document& doc)
    : _delegate(parent, doc)
{
}

sp<Object> Body::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _delegate.build(args);
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
    DCHECK(!_world.world().IsLocked(), "Cannot destroy body in the middle of a time step");
    Shadow* shadow = reinterpret_cast<Shadow*>(_body->GetUserData());
    DASSERT(shadow);
    delete  shadow;

    LOGD("id = %d", _id);
    _body->SetUserData(nullptr);
    _world.world().DestroyBody(_body);
    _body = nullptr;
}

Body::Shadow::Shadow(const sp<Body::Stub>& body, const sp<RigidBody::Stub>& rigidBody)
    : _body(body), _rigid_body(rigidBody), _tag(rigidBody->_tag)
{
}

}
}
}
