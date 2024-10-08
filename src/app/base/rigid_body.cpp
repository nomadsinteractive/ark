#include "app/base/rigid_body.h"

#include "core/base/bean_factory.h"
#include "core/base/ref_manager.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/ref.h"
#include "core/util/string_convert.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"
#include "app/traits/shape.h"

namespace ark {

RigidBody::RigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> quaternion, Box impl, sp<Ref> ref)
    : _ref(ref ? std::move(ref) : Global<RefManager>()->makeRef(this)), _type(type), _meta_id(0), _shape(std::move(shape)), _position(std::move(position)), _quaternion(std::move(quaternion)), _impl(std::move(impl))
{
}


RigidBody::~RigidBody()
{
    _ref->discard();
}

void RigidBody::discard()
{
    _ref->discard();
}

TypeId RigidBody::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void RigidBody::onWire(const WiringContext& context)
{
    if(auto position = context.getComponent<Vec3>())
        _position.reset(std::move(position));

    if(auto shape = context.getComponent<Shape>())
        _shape = std::move(shape);

    if(auto collisionCallback = context.getComponent<CollisionCallback>())
        _collision_callback = std::move(collisionCallback);

    if(sp<Boolean> expendable = context.getComponent<Expendable>())
        _ref->setDiscarded(std::move(expendable));
}

const sp<Ref>& RigidBody::ref() const
{
    return _ref;
}

Collider::BodyType RigidBody::type() const
{
    return _type;
}

Collider::BodyType RigidBody::rigidType() const
{
    return static_cast<Collider::BodyType>(_type & Collider::BODY_TYPE_RIGID);
}

const sp<Shape>& RigidBody::shape() const
{
    return _shape;
}

uint32_t RigidBody::metaId() const
{
    return _meta_id;
}

const SafeVar<Vec3>& RigidBody::position() const
{
    return _position;
}

const SafeVar<Vec4>& RigidBody::quaternion() const
{
    return _quaternion;
}

const SafeVar<Boolean>& RigidBody::discarded() const
{
    return _ref->discarded();
}

const sp<CollisionCallback>& RigidBody::collisionCallback() const
{
    return _collision_callback;
}

void RigidBody::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    _collision_callback = std::move(collisionCallback);
}

const sp<CollisionFilter>& RigidBody::collisionFilter() const
{
    return _collision_filter;
}

void RigidBody::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _collision_filter = std::move(collisionFilter);
}

sp<RigidBody> RigidBody::makeShadow() const
{
    return sp<RigidBody>::make(_type, _shape, _position.wrapped(), _quaternion.wrapped(), _impl, sp<Ref>::make(*_ref));
}

template<> ARK_API Collider::BodyType StringConvert::eval<Collider::BodyType>(const String& str)
{
    if(str == "static")
        return Collider::BODY_TYPE_STATIC;
    if(str == "kinematic")
        return Collider::BODY_TYPE_KINEMATIC;
    if(str == "dynamic")
        return Collider::BODY_TYPE_DYNAMIC;
    FATAL("Unknow body type \"%s\"", str.c_str());
    return Collider::BODY_TYPE_STATIC;
}

void RigidBody::onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) const
{
    if(_collision_callback && rigidBody.type() != Collider::BODY_TYPE_SENSOR)
        _collision_callback->onBeginContact(rigidBody, manifold);
}

void RigidBody::onEndContact(const RigidBody& rigidBody) const
{
    if(_collision_callback && rigidBody.type() != Collider::BODY_TYPE_SENSOR)
        _collision_callback->onEndContact(rigidBody);
}

void RigidBody::onBeginContact(const RigidBody& self, const RigidBody& rigidBody, const CollisionManifold& manifold) const
{
    onBeginContact(rigidBody, manifold);
    if(rigidBody.rigidType() == Collider::BODY_TYPE_STATIC)
        rigidBody.onBeginContact(self, CollisionManifold(manifold.contactPoint(), -manifold.normal()));
}

void RigidBody::onEndContact(const RigidBody& self, const RigidBody& rigidBody) const
{
    onEndContact(rigidBody);
    if(rigidBody.rigidType() == Collider::BODY_TYPE_STATIC)
        rigidBody.onEndContact(self);
}

}
