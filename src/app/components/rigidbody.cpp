#include "app/components/rigidbody.h"

#include "core/base/bean_factory.h"
#include "core/base/ref_manager.h"
#include "core/base/string.h"
#include "core/components/discarded.h"
#include "core/components/tags.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/string_convert.h"

#include "graphics/base/boundaries.h"
#include "graphics/components/rotation.h"
#include "graphics/components/translation.h"

#include "app/base/application_context.h"
#include "app/components/shape.h"
#include "app/inf/collider.h"
#include "app/inf/collision_callback.h"
#include "app/util/collider_type.h"
#include "graphics/base/transform_3d.h"
#include "graphics/components/render_object.h"

namespace ark {

Rigidbody::Stub::Stub(sp<Ref> ref, BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter)
    : _ref(std::move(ref)), _type(type), _shape(std::move(shape)), _position(std::move(position)), _rotation(std::move(rotation)), _collision_filter(std::move(collisionFilter))
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

Rigidbody::Rigidbody(BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded, sp<RigidbodyController> controller, const bool isShadow)
    : _impl{sp<Stub>::make(Global<RefManager>()->makeRef(this, std::move(discarded)), type, std::move(shape), std::move(position), std::move(rotation)), nullptr, std::move(controller)}, _is_shadow(isShadow)
{
}

Rigidbody::Rigidbody(Impl impl, const bool isShadow)
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
    if(sp<Vec3> position = context.getComponent<Translation>())
        _impl._stub->_position.reset(std::move(position));

    if(sp<Vec4> rotation = context.getComponent<Rotation>())
        _impl._stub->_rotation.reset(std::move(rotation));

    if(sp<Boolean> discarded = context.getComponent<Discarded>())
        _impl._stub->_ref->setDiscarded(std::move(discarded));

    if(auto shape = context.getComponent<Shape>())
    {
        const sp<Collider> collider = _impl._collider;
        ASSERT(shape->type().hash() != Shape::TYPE_NONE);
        ASSERT(collider);
        ASSERT(_is_shadow);
        const Impl impl = std::move(_impl);
        _impl = collider->createBody(impl._stub->_type, std::move(shape), impl._stub->_position.wrapped(), impl._stub->_rotation.wrapped(), std::move(impl._stub->_collision_filter), impl._stub->_ref->discarded().wrapped());
        _impl._stub->_collision_callback = std::move(impl._stub->_collision_callback);
        _impl._stub->_collision_filter = std::move(impl._stub->_collision_filter);
        _impl._stub->_tags = std::move(impl._stub->_tags);
        _is_shadow = false;
    }

    if(type() == BODY_TYPE_DYNAMIC)
        if(const sp<RenderObject> renderObject = context.getComponent<RenderObject>())
        {
            renderObject->setPosition(position().toVar());
            renderObject->setTransform(sp<Transform>::make<Transform3D>(rotation().toVar(), nullptr, nullptr));
        }

    if(!_impl._stub->_collision_callback)
        if(auto collisionCallback = context.getComponent<CollisionCallback>())
            _impl._stub->_collision_callback = std::move(collisionCallback);

    if(sp<Tags> tags = context.getComponent<Tags>())
        _impl._stub->_tags = std::move(tags);
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
    return _impl._stub->_tags ? _impl._stub->_tags->tag() : nullptr;
}

void Rigidbody::setTag(Box tag)
{
    if(_impl._stub->_tags)
        _impl._stub->_tags->setTag(0, std::move(tag));
    else
        _impl._stub->_tags = sp<Tags>::make(std::move(tag));
}

const sp<RigidbodyController>& Rigidbody::controller() const
{
    return _impl._controller;
}

sp<Rigidbody> Rigidbody::makeShadow() const
{
    return sp<Rigidbody>::make(_impl, true);
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
    sp<Rigidbody> rigidbody = ColliderType::createBody(collider, _body_type.build(args), _shape.build(args), _position.build(args), _rotation.build(args), _collision_filter.build(args), _discarded.build(args));
    if(sp<CollisionCallback> collisionCallback = _collision_callback.build(args))
        rigidbody->setCollisionCallback(std::move(collisionCallback));
    return rigidbody;
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
    if(str == "ghost")
        return Rigidbody::BODY_TYPE_GHOST;
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

Rigidbody::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _rigidbody(factory.ensureBuilder<Rigidbody>(manifest))
{
}

sp<Wirable> Rigidbody::BUILDER_WIRABLE::build(const Scope& args)
{
    return _rigidbody->build(args);
}

}
