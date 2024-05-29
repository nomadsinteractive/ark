#include "app/base/rigid_body.h"

#include <algorithm>

#include "core/ark.h"

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/string_convert.h"
#include "core/util/holder_util.h"

#include "graphics/base/size.h"
#include "graphics/base/v3.h"
#include "graphics/base/render_object.h"
#include "graphics/util/vec3_type.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"
#include "app/traits/shape.h"

namespace ark {

RigidBody::RigidBody(int32_t id, Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotate, Box impl, SafeVar<Boolean> discarded)
    : _stub(sp<Stub>::make(id, type, 0, std::move(shape), std::move(position), sp<Transform>::make(Transform::TYPE_LINEAR_3D, rotate), std::move(impl), std::move(discarded)))
{
}

RigidBody::RigidBody(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

void RigidBody::bind(const sp<RenderObject>& renderObject)
{
    renderObject->setPosition(_stub->_position);
    renderObject->setTransform(_stub->_transform);
}

int32_t RigidBody::id() const
{
    return _stub->_id;
}

Collider::BodyType RigidBody::type() const
{
    return _stub->_type;
}

Collider::BodyType RigidBody::rigidType() const
{
    return static_cast<Collider::BodyType>(_stub->_type & Collider::BODY_TYPE_RIGID);
}

const sp<Shape>& RigidBody::shape() const
{
    return _stub->_shape;
}

uint32_t RigidBody::metaId() const
{
    return _stub->_meta_id;
}

const sp<Vec3>& RigidBody::position() const
{
    return _stub->_position;
}

const sp<Transform>& RigidBody::transform() const
{
    return _stub->_transform;
}

const sp<CollisionCallback>& RigidBody::collisionCallback() const
{
    return _stub->_callback->_collision_callback;
}

void RigidBody::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    _stub->_callback->_collision_callback = std::move(collisionCallback);
}

const sp<CollisionFilter>& RigidBody::collisionFilter() const
{
    return _stub->_collision_filter;
}

void RigidBody::setCollisionFilter(sp<CollisionFilter> collisionFilter)
{
    _stub->_collision_filter = std::move(collisionFilter);
}

const sp<RigidBody::Stub>& RigidBody::stub() const
{
    return _stub;
}

const sp<RigidBody::Callback>& RigidBody::callback() const
{
    return _stub->_callback;
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

RigidBody::Stub::Stub(int32_t id, Collider::BodyType type, uint32_t metaId, sp<Shape> shape, sp<Vec3> position, sp<Transform> transform, Box impl, SafeVar<Boolean> discarded)
    : _id(id), _type(type), _meta_id(metaId), _shape(std::move(shape)), _position(std::move(position)), _transform(std::move(transform)), _impl(std::move(impl)), _discarded(std::move(discarded)), _callback(sp<Callback>::make())
{
}

RigidBody::Stub::~Stub()
{
    _discarded.reset(true);
    LOGD("RigidBody(%d) disposed", _id);
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
