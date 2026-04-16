#include "app/components/rigidbody.h"

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/components/discarded.h"
#include "core/components/tags.h"
#include "core/inf/variable.h"
#include "core/types/ref.h"
#include "core/util/string_convert.h"

#include "graphics/base/shape.h"
#include "graphics/components/rotation.h"
#include "graphics/components/render_object.h"
#include "graphics/components/translation.h"

#include "app/base/application_context.h"
#include "app/inf/collider.h"
#include "app/inf/collision_callback.h"

namespace ark {

Rigidbody::Stub::Stub(sp<Ref> ref, const BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter)
    : _ref(std::move(ref)), _type(type), _shape(std::move(shape)), _position(std::move(position)), _rotation(std::move(rotation), constants::QUATERNION_ONE), _collision_filter(std::move(collisionFilter))
{
}

void Rigidbody::Stub::onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) const
{
    if(_collision_callback && rigidBody.type() != BODY_TYPE_SENSOR)
        _collision_callback->onBeginContact(rigidBody, manifold);
}

void Rigidbody::Stub::onEndContact(const Rigidbody& rigidBody) const
{
    if(_collision_callback && rigidBody.type() != BODY_TYPE_SENSOR)
        _collision_callback->onEndContact(rigidBody);
}

Rigidbody::Rigidbody(sp<Stub> stub, sp<RigidbodyController> controller, const bool isShadow)
    : _stub(std::move(stub)), _controller(std::move(controller)), _is_shadow(isShadow)
{
}

Rigidbody::~Rigidbody()
{
    if(!_is_shadow)
        _stub->_ref->discard();
}

void Rigidbody::discard()
{
    _stub->_ref->discard();
}

void Rigidbody::onWire(const WiringContext& context, const Box& self)
{
    if(type() == BODY_TYPE_DYNAMIC)
    {
        if(const sp<Translation> translation = context.getInterface<Translation>())
            translation->reset(_stub->_position.toVar());
        if(const sp<Rotation> rotation = context.getInterface<Rotation>())
            rotation->reset(_stub->_rotation.toVar());
    }
    else
    {
        if(sp<Vec3> position = context.getComponent<Translation>())
            _stub->_position.reset(std::move(position));
        if(sp<Vec4> rotation = context.getComponent<Rotation>())
            _stub->_rotation.reset(std::move(rotation));
    }

    if(sp<Boolean> discarded = context.getComponent<Discarded>())
        _stub->_ref->setDiscarded(std::move(discarded));

    if(!_stub->_collision_callback)
        if(auto collisionCallback = context.getComponent<CollisionCallback>())
            _stub->_collision_callback = std::move(collisionCallback);

    if(sp<Tags> tags = context.getComponent<Tags>())
        _stub->_tags = std::move(tags);
}

RefId Rigidbody::id() const
{
    return _stub->_ref->id();
}

Rigidbody::BodyType Rigidbody::type() const
{
    return _stub->_type;
}

const sp<Shape>& Rigidbody::shape() const
{
    return _stub->_shape;
}

const OptionalVar<Vec3>& Rigidbody::position() const
{
    return _stub->_position;
}

const OptionalVar<Vec4>& Rigidbody::rotation() const
{
    return _stub->_rotation;
}

const OptionalVar<Boolean>& Rigidbody::discarded() const
{
    return _stub->_ref->discarded();
}

const sp<CollisionCallback>& Rigidbody::collisionCallback() const
{
    return _stub->_collision_callback;
}

void Rigidbody::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    _stub->_collision_callback = std::move(collisionCallback);
}

const sp<CollisionFilter>& Rigidbody::collisionFilter() const
{
    return _stub->_collision_filter;
}

void Rigidbody::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _stub->_collision_filter = std::move(collisionFilter);
}

Box Rigidbody::tag() const
{
    return _stub->_tags ? _stub->_tags->tag() : nullptr;
}

void Rigidbody::setTag(Box tag)
{
    if(_stub->_tags)
        _stub->_tags->setTag(0, std::move(tag));
    else
        _stub->_tags = sp<Tags>::make(std::move(tag));
}

const sp<RigidbodyController>& Rigidbody::controller() const
{
    return _controller;
}

sp<Rigidbody> Rigidbody::makeShadow() const
{
    return sp<Rigidbody>::make(_stub, _controller, true);
}

Rigidbody::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _collider(factory.ensureBuilder<Collider>(manifest, "collider")), _body_type(manifest, "body-type", BODY_TYPE_KINEMATIC), _shape(factory.getBuilder<Shape>(manifest, "shape")),
      _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)), _rotation(factory.getBuilder<Vec4>(manifest, constants::ROTATION)), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED)),
      _collision_callback(factory.getBuilder<CollisionCallback>(manifest, constants::COLLISION_CALLBACK)), _collision_filter(factory.getBuilder<CollisionFilter>(manifest, "collision-filter"))
{
}

sp<Rigidbody> Rigidbody::BUILDER::build(const Scope& args)
{
    const sp<Collider> collider = _collider->build(args);
    sp<Rigidbody> rigidbody = Collider::createBody(collider, _body_type.build(args), _shape.build(args), _position.build(args), _rotation.build(args), _collision_filter.build(args), _discarded.build(args));
    if(sp<CollisionCallback> collisionCallback = _collision_callback.build(args))
        rigidbody->setCollisionCallback(std::move(collisionCallback));
    return rigidbody;
}

template<> ARK_API Rigidbody::BodyType StringConvert::eval<Rigidbody::BodyType>(const String& str)
{
    constexpr enums::LookupTable<Rigidbody::BodyType, 5> table = {{
        {"static", Rigidbody::BODY_TYPE_STATIC},
        {"kinematic", Rigidbody::BODY_TYPE_KINEMATIC},
        {"dynamic", Rigidbody::BODY_TYPE_DYNAMIC},
        {"sensor", Rigidbody::BODY_TYPE_SENSOR},
        {"ghost", Rigidbody::BODY_TYPE_GHOST}
    }};
    return enums::lookup(table, str);
}

void Rigidbody::onBeginContact(const Rigidbody& rigidbody, const CollisionManifold& manifold) const
{
    _stub->onBeginContact(rigidbody, manifold);
}

void Rigidbody::onEndContact(const Rigidbody& rigidbody) const
{
    _stub->onEndContact(rigidbody);
}

Rigidbody::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _rigidbody(factory.ensureBuilder<Rigidbody>(manifest))
{
}

sp<Wirable> Rigidbody::BUILDER_WIRABLE::build(const Scope& args)
{
    return _rigidbody->build(args);
}

}
