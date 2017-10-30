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

}

Body::Body(const sp<World>& world, Type type, float x, float y, Shape& shape, float density, float friction)
    : _world(world), _body(world->createBody(type, x, y, shape, density, friction))
{
}

Body::Body(const sp<World>& world, b2Body* body)
    : _world(world), _body(body)
{
}

Body::~Body()
{
    _world->world().DestroyBody(_body);
}

const sp<World>& Body::world() const
{
    return _world;
}

float Body::rotation()
{
    return _body->GetAngle();
}

void Body::setRotation(float rad)
{
    _body->SetTransform(_body->GetWorldCenter(), rad);
}

float Body::angularVelocity()
{
    return _body->GetAngularVelocity();
}

void Body::setAngularVelocity(float omega)
{
    _body->SetAngularVelocity(omega);
}

bool Body::active()
{
    return _body->IsActive();
}

void Body::setActive(bool active)
{
    _body->SetActive(active);
}

bool Body::awake()
{
    return _body->IsAwake();
}

void Body::setAwake(bool awake)
{
    _body->SetAwake(awake);
}

float Body::x() const
{
    return _body->GetPosition().x;
}

float Body::y() const
{
    return _body->GetPosition().y;
}

float Body::mass() const
{
    return _body->GetMass();
}

void Body::applyTorque(float torque)
{
    _body->ApplyTorque(torque, true);
}

void Body::applyForce(const sp<VV2>& force, const sp<VV2>& point)
{
    const V2 f = force->val();
    const V2 p = point->val();
    _body->ApplyForce(b2Vec2(f.x(), f.y()), b2Vec2(p.x(), p.y()), true);
}

void Body::applyForceToCenter(const sp<VV2>& force)
{
    const V2 f = force->val();
    _body->ApplyForceToCenter(b2Vec2(f.x(), f.y()), true);
}

void Body::applyLinearImpulse(const sp<VV2>& impulse, const sp<VV2>& point)
{
    const V2 i = impulse->val();
    const V2 p = point->val();
    _body->ApplyLinearImpulse(b2Vec2(i.x(), i.y()), b2Vec2(p.x(), p.y()), true);
}

void Body::applyAngularImpulse(float impulse)
{
    _body->ApplyAngularImpulse(impulse, true);
}

b2Body* Body::b2Instance() const
{
    return _body;
}

Body::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& parent, const document& doc)
    : _world(parent.ensureBuilder<Object>(doc, "world")),
      _shape(parent.ensureBuilder<Shape>(doc, "shape")),
      _position(parent.ensureBuilder<VV2>(doc, Constants::Attributes::POSITION)),
      _density(Documents::getAttribute<float>(doc, "density", 1.0f)),
      _friction(Documents::getAttribute<float>(doc, "friction", 0.2f))
{
    DCHECK(_world, "Illegal world object");
}

sp<Body> Body::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    const sp<World> world = BeanUtils::as<World>(_world, args);
    const sp<Shape> shape = _shape->build(args);
    const V2 p = _position->build(args)->val();
    return sp<Body>::make(world, Body::BODY_TYPE_DYNAMIC, p.x(), p.y(), shape, _density, _friction);
}

Body::POSITION_BUILDER::POSITION_BUILDER(BeanFactory& parent, const document& doc)
    : _body(parent.ensureBuilder<Object>(doc, "body"))
{
}

sp<Vec2> Body::POSITION_BUILDER::build(const sp<Scope>& args)
{
    const sp<Body> body = _body->build(args).as<Body>();
    DCHECK(body, "Illegal body object");
    return sp<Vec2>::make(sp<_PositionX>::make(body), sp<_PositionY>::make(body));
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

Body::RENDER_OBJECT_DECORATOR::RENDER_OBJECT_DECORATOR(BeanFactory& parent, const sp<Builder<RenderObject>>& delegate, const String& value)
    : _delegate(delegate), _body(parent.ensureBuilder<Object>(value))
{
}

sp<RenderObject> Body::RENDER_OBJECT_DECORATOR::build(const sp<Scope>& args)
{
    const sp<Body> body = _body->build(args).as<Body>();
    NOT_NULL(body);
    sp<RenderObject> decorated = _delegate->build(args);
    decorated->setPosition(sp<Vec2>::make(sp<_PositionX>::make(body), sp<_PositionY>::make(body)));
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

}
}

template<> plugin::box2d::Body::Type Conversions::to<String, plugin::box2d::Body::Type>(const String& str)
{
    if(str == "static")
        return plugin::box2d::Body::BODY_TYPE_STATIC;
    if(str == "kinematic")
        return plugin::box2d::Body::BODY_TYPE_KINEMATIC;
    if(str == "dynamic")
        return plugin::box2d::Body::BODY_TYPE_DYNAMIC;
    FATAL("Unknow body type \"%s\"", str.c_str());
    return plugin::box2d::Body::BODY_TYPE_STATIC;
}

}
