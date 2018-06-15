#include "box2d/impl/body.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
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

class DynamicBodyRotation : public Numeric {
public:
    DynamicBodyRotation(const sp<Body::Stub>& stub, const sp<Numeric>& delegate)
        : _stub(stub), _delegate(delegate) {
    }

    virtual float val() override {
        DCHECK(_stub->_body, "Body has been disposed already");
        return _stub->_body->GetAngle();
    }

    sp<Body::Stub> _stub;
    sp<Numeric> _delegate;
};

class DynamicBodyPosition : public Vec {
public:
    DynamicBodyPosition(const sp<Body::Stub>& stub, const sp<Vec>& delegate)
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

    sp<Body::Stub> _stub;
};


class KinematicBodyPosition : public Vec {
public:
    KinematicBodyPosition(const sp<Body::Stub>& stub, const sp<Vec>& delegate)
        : _stub(stub), _delegate(delegate) {
    }

    virtual V val() override {
        DCHECK(_stub->_body, "Body has been disposed");
        V pos = _delegate->val();
        return V(_stub->_world.toPixelX(pos.x()), _stub->_world.toPixelY(pos.y()));
    }

private:
    sp<Body::Stub> _stub;
    sp<Vec> _delegate;
};

class KinematicRotation : public Numeric {
public:
    KinematicRotation(const sp<Body::Stub>& stub, const sp<Numeric>& delegate)
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

Body::Body(const World& world, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotation, Shape& shape, float density, float friction)
    : Body(sp<Stub>::make(world, world.createBody(type, position->val(), size, shape, density, friction)), type, position, size, rotation)
{
}

Body::Body(const sp<Stub>& stub, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Numeric>& rotation)
    : RigidBody(stub->_id, type,
                sp<DynamicBodyPosition>::make(stub, position),
                size,
                sp<Rotate>::make(sp<DynamicBodyRotation>::make(stub, rotation))), _stub(stub)
{
    _stub->_callback = callback();
}

void Body::bind(const sp<RenderObject>& renderObject)
{
    float ppmx = world().toPixelX(1.0f);
    float ppmy = world().toPixelY(1.0f);
    if(type() == Collider::BODY_TYPE_KINEMATIC)
    {
        const sp<Numeric> rotation = rotate() ? NumericUtil::delegate(rotate()->rotation()).cast<DynamicBodyRotation>()->_delegate : sp<Numeric>::null();
        renderObject->transform()->rotate()->setRotation(sp<KinematicRotation>::make(_stub, rotation));
    }
    else
    {
        renderObject->setPosition(sp<RenderObjectPosition>::make(_stub));
        renderObject->setSize(sp<Size>::make(NumericUtil::mul(size()->vwidth(), sp<Numeric::Const>::make(ppmx)),
                                             NumericUtil::mul(size()->vheight(), sp<Numeric::Const>::make(ppmy))));
        renderObject->transform()->setRotate(rotate());
    }
}

void Body::dispose()
{
    _stub->dispose();
}

const World& Body::world() const
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
    body->SetUserData(this);
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
    _body->SetUserData(nullptr);
    _world.world().DestroyBody(_body);
    _body = nullptr;
}

}
}
}
