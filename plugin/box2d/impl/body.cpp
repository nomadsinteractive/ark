#include "box2d/impl/body.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/bean_utils.h"

#include "graphics/base/bounds.h"
#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"
#include "graphics/base/vec2.h"

#include "impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

namespace {

class _PositionX : public Numeric {
public:
    _PositionX(const sp<Body>& body)
        : _world(body->world()), _body(body) {
    }

    virtual float val() override {
        return _world->toPixelX(_body->b2Instance()->GetPosition().x);
    }

private:
    sp<World> _world;
    sp<Body> _body;
};

class _PositionY : public Numeric {
public:
    _PositionY(const sp<Body>& body)
        : _world(body->world()), _body(body) {
    }

    virtual float val() override {
        return _world->toPixelY(_body->b2Instance()->GetPosition().y);
    }

private:
    sp<World> _world;
    sp<Body> _body;
};

class _Angle : public Numeric {
public:
    _Angle(const sp<Body>& body)
        : _world(body->world()), _body(body) {
    }

    virtual float val() override {
        return _body->b2Instance()->GetAngle();
    }

private:
    sp<World> _world;
    sp<Body> _body;
};

class RigidBodyPosition : public VV {
public:
    RigidBodyPosition(const sp<Body::Stub>& stub)
        : _stub(stub) {
    }

    virtual V val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        float x = _stub->_world->toPixelX(_stub->_body->GetPosition().x);
        float y = _stub->_world->toPixelX(_stub->_body->GetPosition().y);
        return V(x, y);
    }

private:
    sp<Body::Stub> _stub;
};

}

Body::Body(const sp<World>& world, Collider::BodyType type, float x, float y, Shape& shape, float density, float friction)
    : Body(sp<Stub>::make(world, world->createBody(type, x, y, shape, density, friction)))
{
}

Body::Body(const sp<World>& world, b2Body* body)
    : Body(sp<Stub>::make(world, body))
{
}

Body::Body(const sp<Stub>& stub)
    : RigidBody(reinterpret_cast<uint32_t>(_stub->_body), Collider::BODY_TYPE_DYNAMIC, nullptr, nullptr, nullptr), _stub(stub)
{
}

Body::~Body()
{
    if(_stub->_body)
        dispose();
}

void Body::dispose()
{
    DCHECK(_stub->_body, "Body has been disposed already");
    _stub->_world->world().DestroyBody(_stub->_body);
    _stub->_body = nullptr;
}

const sp<CollisionCallback>& Body::collisionCallback() const
{
    return _stub->_collision_callback;
}

void Body::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _stub->_collision_callback = collisionCallback;
}

const sp<World>& Body::world() const
{
    return _stub->_world;
}

float Body::rotation()
{
    return _stub->_body->GetAngle();
}

void Body::setRotation(float rad)
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

bool Body::active()
{
    return _stub->_body->IsActive();
}

void Body::setActive(bool active)
{
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

void Body::applyTorque(float torque)
{
    _stub->_body->ApplyTorque(torque, true);
}

void Body::applyForce(const sp<VV>& force, const sp<VV>& point)
{
    const V f = force->val();
    const V p = point->val();
    _stub->_body->ApplyForce(b2Vec2(f.x(), f.y()), b2Vec2(p.x(), p.y()), true);
}

void Body::applyForceToCenter(const sp<VV>& force)
{
    const V f = force->val();
    _stub->_body->ApplyForceToCenter(b2Vec2(f.x(), f.y()), true);
}

void Body::applyLinearImpulse(const sp<VV>& impulse, const sp<VV>& point)
{
    const V i = impulse->val();
    const V p = point->val();
    _stub->_body->ApplyLinearImpulse(b2Vec2(i.x(), i.y()), b2Vec2(p.x(), p.y()), true);
}

void Body::applyAngularImpulse(float impulse)
{
    _stub->_body->ApplyAngularImpulse(impulse, true);
}

b2Body* Body::b2Instance() const
{
    return _stub->_body;
}

Body::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& parent, const document& doc)
    : _world(parent.ensureBuilder<Object>(doc, "world")),
      _shape(parent.ensureBuilder<Shape>(doc, "shape")),
      _position(parent.ensureBuilder<VV>(doc, Constants::Attributes::POSITION)),
      _density(Documents::getAttribute<float>(doc, "density", 1.0f)),
      _friction(Documents::getAttribute<float>(doc, "friction", 0.2f))
{
    DCHECK(_world, "Illegal world object");
}

sp<Body> Body::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    const sp<World> world = BeanUtils::as<World>(_world, args);
    const sp<Shape> shape = _shape->build(args);
    const V p = _position->build(args)->val();
    return sp<Body>::make(world, Collider::BODY_TYPE_DYNAMIC, p.x(), p.y(), shape, _density, _friction);
}

Body::POSITION_BUILDER::POSITION_BUILDER(BeanFactory& parent, const document& doc)
    : _body(parent.ensureBuilder<Object>(doc, "body"))
{
}

sp<Vec> Body::POSITION_BUILDER::build(const sp<Scope>& args)
{
    const sp<Body> body = _body->build(args).as<Body>();
    DCHECK(body, "Illegal body object");
    return sp<Vec>::make(sp<_PositionX>::make(body), sp<_PositionY>::make(body));
}

Body::ROTATION_BUILDER::ROTATION_BUILDER(BeanFactory& parent, const document& doc)
    : _body(parent.ensureBuilder<Object>(doc, "body"))
{
}

sp<Numeric> Body::ROTATION_BUILDER::build(const sp<Scope>& args)
{
    const sp<Body> body = _body->build(args).as<Body>();
    DCHECK(body, "Illegal body object");
    return sp<_Angle>::make(body);
}

Body::RENDER_OBJECT_STYLE::RENDER_OBJECT_STYLE(BeanFactory& parent, const sp<Builder<RenderObject>>& delegate, const String& value)
    : _delegate(delegate), _body(parent.ensureBuilder<Object>(value))
{
}

sp<RenderObject> Body::RENDER_OBJECT_STYLE::build(const sp<Scope>& args)
{
    const sp<Body> body = _body->build(args).as<Body>();
    NOT_NULL(body);
    sp<RenderObject> decorated = _delegate->build(args);
    decorated->setPosition(sp<Vec>::make(sp<_PositionX>::make(body), sp<_PositionY>::make(body)));
    decorated->transform()->setRotation(sp<_Angle>::make(body));
    decorated.absorb(body);
    return decorated;
}

Body::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& parent, const document& doc)
    : _delegate(parent, doc)
{
}

sp<Object> Body::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

Body::Stub::Stub(const sp<World>& world, b2Body* body)
    : _world(world), _body(body)
{
}

}
}
}
