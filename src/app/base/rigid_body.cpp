#include "app/base/rigid_body.h"

#include <algorithm>

#include "core/ark.h"

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/string_convert.h"
#include "core/util/holder_util.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"
#include "app/traits/shape.h"
#include "core/types/ref.h"

namespace ark {

RigidBody::RigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotation, Box impl, SafeVar<Boolean> discarded)
    : _ref(sp<RigidBodyRef>::make(*this)), _type(type), _meta_id(0), _shape(std::move(shape)), _position(std::move(position)), _rotation(std::move(rotation)), _impl(std::move(impl)), _discarded(std::move(discarded)), _callback(sp<Callback>::make())
{
}

RigidBody::~RigidBody()
{
    LOGD("RigidBody(%uz) disposed", id());
    _ref->discard();
}

std::vector<std::pair<TypeId, Box>> RigidBody::onWire(const Traits& components)
{
    if(sp<Vec3> position = components.get<Vec3>())
        _position.reset(std::move(position));

    if(sp<Shape> shape = components.get<Shape>())
    {
        _shape = std::move(shape);
        _timestamp.markDirty();
    }

    return {};
}

uintptr_t RigidBody::id() const
{
    return reinterpret_cast<uintptr_t>(_ref.get());
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

const sp<Vec3>& RigidBody::position() const
{
    return _position.wrapped();
}

const sp<Rotation>& RigidBody::rotation() const
{
    return _rotation;
}

const sp<CollisionCallback>& RigidBody::collisionCallback() const
{
    return _callback->_collision_callback;
}

void RigidBody::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    _callback->_collision_callback = std::move(collisionCallback);
}

const sp<CollisionFilter>& RigidBody::collisionFilter() const
{
    return _collision_filter;
}

void RigidBody::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _collision_filter = std::move(collisionFilter);
}

const sp<RigidBody::Callback>& RigidBody::callback() const
{
    return _callback;
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

void RigidBody::Callback::onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    if(_collision_callback && rigidBody->type() != Collider::BODY_TYPE_SENSOR)
        _collision_callback->onBeginContact(rigidBody, manifold);
}

void RigidBody::Callback::onEndContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback && rigidBody->type() != Collider::BODY_TYPE_SENSOR)
        _collision_callback->onEndContact(rigidBody);
}

void RigidBody::Callback::onBeginContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    onBeginContact(rigidBody, manifold);
    if(rigidBody->rigidType() == Collider::BODY_TYPE_STATIC)
        rigidBody->callback()->onBeginContact(self, CollisionManifold(manifold.contactPoint(), -manifold.normal()));
}

void RigidBody::Callback::onEndContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody)
{
    onEndContact(rigidBody);
    if(rigidBody->rigidType() == Collider::BODY_TYPE_STATIC)
        rigidBody->callback()->onEndContact(self);
}

bool RigidBody::Callback::hasCallback() const
{
    return static_cast<bool>(_collision_callback);
}

}
