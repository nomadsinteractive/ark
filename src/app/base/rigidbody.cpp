#include "app/base/rigidbody.h"

#include "core/base/bean_factory.h"
#include "core/base/ref_manager.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/string_convert.h"

#include "graphics/base/boundaries.h"
#include "graphics/components/position.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"
#include "app/components/shape.h"
#include "app/components/with_tag.h"

namespace ark {

Rigidbody::Rigidbody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, Box impl, sp<Ref> ref, bool isShadow)
    : _ref(ref ? std::move(ref) : Global<RefManager>()->makeRef(this)), _type(type), _shape(std::move(shape)), _position(std::move(position)), _rotation(std::move(rotation), constants::QUATERNION_ONE), _impl(std::move(impl)), _is_shadow(isShadow)
{
}

Rigidbody::~Rigidbody()
{
    if(!_is_shadow)
        _ref->discard();
}

void Rigidbody::discard()
{
    _ref->discard();
}

void Rigidbody::onWire(const WiringContext& context)
{
    if(sp<Vec3> position = context.getComponent<Position>())
        _position.reset(std::move(position));

    if(auto shape = context.getComponent<Shape>())
        _shape = std::move(shape);

    if(auto collisionCallback = context.getComponent<CollisionCallback>())
        _collision_callback = std::move(collisionCallback);

    if(sp<Boolean> discarded = context.getComponent<Discarded>())
        _ref->setDiscarded(std::move(discarded));

    if(sp<WithTag> withTag = context.getComponent<WithTag>())
        _with_tag = std::move(withTag);
}

RefId Rigidbody::id() const
{
    return _ref->id();
}

const sp<Ref>& Rigidbody::ref() const
{
    return _ref;
}

Collider::BodyType Rigidbody::type() const
{
    return _type;
}

const sp<Shape>& Rigidbody::shape() const
{
    return _shape;
}

const SafeVar<Vec3>& Rigidbody::position() const
{
    return _position;
}

const SafeVar<Vec4>& Rigidbody::rotation() const
{
    return _rotation;
}

const SafeVar<Boolean>& Rigidbody::discarded() const
{
    return _ref->discarded();
}

const sp<CollisionCallback>& Rigidbody::collisionCallback() const
{
    return _collision_callback;
}

void Rigidbody::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    _collision_callback = std::move(collisionCallback);
}

const sp<CollisionFilter>& Rigidbody::collisionFilter() const
{
    return _collision_filter;
}

void Rigidbody::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _collision_filter = std::move(collisionFilter);
}

Box Rigidbody::tag() const
{
    return _with_tag ? nullptr : _with_tag->tag();
}

void Rigidbody::setTag(Box tag)
{
    if(_with_tag)
        _with_tag->setTag(std::move(tag));
    else
        _with_tag = sp<WithTag>::make(std::move(tag));
}

sp<Rigidbody> Rigidbody::makeShadow() const
{
    return sp<Rigidbody>::make(_type, _shape, _position.wrapped(), _rotation.wrapped(), _impl, _ref, true);
}

Rigidbody::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _collider(factory.ensureBuilder<Collider>(manifest, "collider")), _body_type(Documents::getAttribute<Collider::BodyType>(manifest, "body-type", Collider::BODY_TYPE_KINEMATIC)), _shape(factory.getBuilder<Shape>(manifest, "shape")),
      _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)), _rotation(factory.getBuilder<Vec4>(manifest, constants::ROTATION)), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED))
{
}

sp<Rigidbody> Rigidbody::BUILDER::build(const Scope& args)
{
    const sp<Collider> collider = _collider->build(args);
    sp<Shape> shape = _shape.build(args);
    return collider->createBody(_body_type, shape ? std::move(shape) : sp<Shape>::make(), _position.build(args), _rotation.build(args), _discarded.build(args));
}

template<> ARK_API Collider::BodyType StringConvert::eval<Collider::BodyType>(const String& str)
{
    if(str == "static")
        return Collider::BODY_TYPE_STATIC;
    if(str == "kinematic")
        return Collider::BODY_TYPE_KINEMATIC;
    if(str == "dynamic")
        return Collider::BODY_TYPE_DYNAMIC;
    if(str == "sensor")
        return Collider::BODY_TYPE_SENSOR;
    FATAL("Unknow body type \"%s\"", str.c_str());
    return Collider::BODY_TYPE_STATIC;
}

void Rigidbody::onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) const
{
    if(_collision_callback && rigidBody.type() != Collider::BODY_TYPE_SENSOR)
        _collision_callback->onBeginContact(rigidBody, manifold);
}

void Rigidbody::onEndContact(const Rigidbody& rigidBody) const
{
    if(_collision_callback && rigidBody.type() != Collider::BODY_TYPE_SENSOR)
        _collision_callback->onEndContact(rigidBody);
}

void Rigidbody::onBeginContact(const Rigidbody& self, const Rigidbody& rigidBody, const CollisionManifold& manifold) const
{
    onBeginContact(rigidBody, manifold);
    if(rigidBody.type() == Collider::BODY_TYPE_STATIC)
        rigidBody.onBeginContact(self, CollisionManifold(manifold.contactPoint(), -manifold.normal()));
}

void Rigidbody::onEndContact(const Rigidbody& self, const Rigidbody& rigidBody) const
{
    onEndContact(rigidBody);
    if(rigidBody.type() == Collider::BODY_TYPE_STATIC)
        rigidBody.onEndContact(self);
}

}
