#include "app/components/rigidbody.h"

#include "core/base/bean_factory.h"
#include "core/base/ref_manager.h"
#include "core/base/string.h"
#include "core/components/with_tag.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/string_convert.h"

#include "graphics/base/boundaries.h"
#include "graphics/components/position.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/components/shape.h"
#include "app/inf/collider.h"
#include "app/inf/collision_callback.h"
#include "app/util/collider_type.h"

namespace ark {

Rigidbody::Stub::Stub(sp<Ref> ref, BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation)
    : _ref(std::move(ref)), _type(type), _shape(std::move(shape)), _position(std::move(position)), _rotation(std::move(rotation))
{
}

void Rigidbody::Stub::onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) const
{
    if(_collision_callback && rigidBody.type() != Rigidbody::BODY_TYPE_SENSOR)
        _collision_callback->onBeginContact(rigidBody, manifold);
}

void Rigidbody::Stub::onEndContact(const Rigidbody& rigidBody) const
{
    if(_collision_callback && rigidBody.type() != Rigidbody::BODY_TYPE_SENSOR)
        _collision_callback->onEndContact(rigidBody);
}

void Rigidbody::Stub::onBeginContact(const Rigidbody& self, const Rigidbody& rigidBody, const CollisionManifold& manifold) const
{
    onBeginContact(rigidBody, manifold);
    if(rigidBody.type() == BODY_TYPE_STATIC)
        rigidBody.onBeginContact(self, CollisionManifold(manifold.contactPoint(), -manifold.normal()));
}

void Rigidbody::Stub::onEndContact(const Rigidbody& self, const Rigidbody& rigidBody) const
{
    onEndContact(rigidBody);
    if(rigidBody.type() == BODY_TYPE_STATIC)
        rigidBody.onEndContact(self);
}

Rigidbody::Rigidbody(BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded, Box impl, bool isShadow)
    : _impl{sp<Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, std::move(shape), std::move(position), std::move(rotation)), std::move(impl)}, _is_shadow(isShadow)
{
}

Rigidbody::Rigidbody(Impl impl, bool isShadow)
    : _impl(std::move(impl)), _is_shadow(isShadow)
{
}

Rigidbody::~Rigidbody()
{
    if(!_is_shadow)
        _impl._stub->_ref->discard();
}

void Rigidbody::discard()
{
    _impl._stub->_ref->discard();
}

void Rigidbody::onWire(const WiringContext& context, const Box& self)
{
    if(auto shape = context.getComponent<Shape>())
    {
        const sp<Collider> collider = _impl._instance.as<Collider>();
        ASSERT(shape->type().hash() != Shape::TYPE_NONE);
        ASSERT(collider);
        ASSERT(_is_shadow);
        _impl = collider->createBody(_impl._stub->_type, std::move(shape), _impl._stub->_position.wrapped(), _impl._stub->_rotation.wrapped(), _impl._stub->_ref->discarded().wrapped());
        _is_shadow = false;
    }
    if(sp<Vec3> position = context.getComponent<Position>())
        _impl._stub->_position.reset(std::move(position));

    if(auto collisionCallback = context.getComponent<CollisionCallback>())
        _impl._stub->_collision_callback = std::move(collisionCallback);

    if(sp<Boolean> discarded = context.getComponent<Discarded>())
        _impl._stub->_ref->setDiscarded(std::move(discarded));

    if(sp<WithTag> withTag = context.getComponent<WithTag>())
        _impl._stub->_with_tag = std::move(withTag);
}

RefId Rigidbody::id() const
{
    return _impl._stub->_ref->id();
}

Rigidbody::BodyType Rigidbody::type() const
{
    return _impl._stub->_type;
}

const sp<Shape>& Rigidbody::shape() const
{
    return _impl._stub->_shape;
}

const SafeVar<Vec3>& Rigidbody::position() const
{
    return _impl._stub->_position;
}

const SafeVar<Vec4>& Rigidbody::rotation() const
{
    return _impl._stub->_rotation;
}

const SafeVar<Boolean>& Rigidbody::discarded() const
{
    return _impl._stub->_ref->discarded();
}

const sp<CollisionCallback>& Rigidbody::collisionCallback() const
{
    return _impl._stub->_collision_callback;
}

void Rigidbody::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    _impl._stub->_collision_callback = std::move(collisionCallback);
}

const sp<CollisionFilter>& Rigidbody::collisionFilter() const
{
    return _impl._stub->_collision_filter;
}

void Rigidbody::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _impl._stub->_collision_filter = std::move(collisionFilter);
}

Box Rigidbody::tag() const
{
    return _impl._stub->_with_tag ? nullptr : _impl._stub->_with_tag->tag();
}

void Rigidbody::setTag(Box tag)
{
    if(_impl._stub->_with_tag)
        _impl._stub->_with_tag->setTag(std::move(tag));
    else
        _impl._stub->_with_tag = sp<WithTag>::make(std::move(tag));
}

const Box& Rigidbody::impl() const
{
    return _impl._instance;
}

sp<Rigidbody> Rigidbody::makeShadow() const
{
    return sp<Rigidbody>::make(_impl, true);
}

Rigidbody::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _collider(factory.ensureBuilder<Collider>(manifest, "collider")), _body_type(Documents::getAttribute<BodyType>(manifest, "body-type", BODY_TYPE_KINEMATIC)), _shape(factory.getBuilder<Shape>(manifest, "shape")),
      _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)), _rotation(factory.getBuilder<Vec4>(manifest, constants::ROTATION)), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED))
{
}

sp<Rigidbody> Rigidbody::BUILDER::build(const Scope& args)
{
    const sp<Collider> collider = _collider->build(args);
    return ColliderType::createBody(collider, _body_type, _shape.build(args), _position.build(args), _rotation.build(args), _discarded.build(args));
}

template<> ARK_API Rigidbody::BodyType StringConvert::eval<Rigidbody::BodyType>(const String& str)
{
    if(str == "static")
        return Rigidbody::BODY_TYPE_STATIC;
    if(str == "kinematic")
        return Rigidbody::BODY_TYPE_KINEMATIC;
    if(str == "dynamic")
        return Rigidbody::BODY_TYPE_DYNAMIC;
    if(str == "sensor")
        return Rigidbody::BODY_TYPE_SENSOR;
    FATAL("Unknow body type \"%s\"", str.c_str());
    return Rigidbody::BODY_TYPE_STATIC;
}

void Rigidbody::onBeginContact(const Rigidbody& rigidbody, const CollisionManifold& manifold) const
{
    _impl._stub->onBeginContact(rigidbody, manifold);
}

void Rigidbody::onEndContact(const Rigidbody& rigidbody) const
{
    _impl._stub->onEndContact(rigidbody);
}

void Rigidbody::onBeginContact(const Rigidbody& self, const Rigidbody& rigidbody, const CollisionManifold& manifold) const
{
    _impl._stub->onBeginContact(self, rigidbody, manifold);
}

void Rigidbody::onEndContact(const Rigidbody& self, const Rigidbody& rigidbody) const
{
    _impl._stub->onEndContact(self, rigidbody);
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
