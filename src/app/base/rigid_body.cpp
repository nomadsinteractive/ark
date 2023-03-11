#include "app/base/rigid_body.h"

#include <algorithm>
#include <iterator>

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

namespace ark {

RigidBody::RigidBody(int32_t id, Collider::BodyType type, int32_t shapeId, sp<Vec3> position, sp<Size> size, sp<Rotation> rotate, Box impl, sp<Disposed> disposed)
    : _stub(sp<Stub>::make(id, type, 0, shapeId, std::move(position), std::move(size), sp<Transform>::make(Transform::TYPE_LINEAR_3D, rotate), std::move(impl), std::move(disposed)))
{
}

RigidBody::RigidBody(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

void RigidBody::bind(const sp<RenderObject>& renderObject)
{
    renderObject->setPosition(position());
    renderObject->setTransform(_stub->_transform);
    setRenderObject(renderObject);
}

void RigidBody::traverse(const Holder::Visitor& visitor)
{
    visitor(_stub->_tag);
    HolderUtil::visit(_stub->_callback->_collision_callback, visitor);
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

int32_t RigidBody::shapeId() const
{
    return _stub->_shape_id;
}

uint32_t RigidBody::metaId() const
{
    return _stub->_meta_id;
}

V2 RigidBody::xy() const
{
    return _stub->_position->val();
}

V3 RigidBody::xyz() const
{
    return _stub->_position->val();
}

float RigidBody::width() const
{
    return _stub->_size->widthAsFloat();
}

float RigidBody::height() const
{
    return _stub->_size->heightAsFloat();
}

const sp<Vec3>& RigidBody::position() const
{
    return _stub->_position;
}

const sp<Size>& RigidBody::size() const
{
    return _stub->_size;
}

const sp<Transform>& RigidBody::transform() const
{
    return _stub->_transform;
}

const Box& RigidBody::tag() const
{
    return _stub->_tag;
}

void RigidBody::setTag(const Box& box) const
{
    _stub->_tag = box;
}

sp<RenderObject> RigidBody::renderObject() const
{
    return _stub->_render_object.lock();
}

void RigidBody::setRenderObject(const sp<RenderObject>& renderObject)
{
    _stub->_render_object = renderObject;
}

const sp<Disposed>& RigidBody::disposed() const
{
    return _stub->_disposed;
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

template<> ARK_API Collider::BodyType StringConvert::to<String, Collider::BodyType>(const String& str)
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

RigidBody::Stub::Stub(int32_t id, Collider::BodyType type, uint32_t metaId, int32_t shapeId, sp<Vec3> position, sp<Size> size, sp<Transform> transform, Box impl, sp<Disposed> disposed)
    : _id(id), _type(type), _meta_id(metaId), _shape_id(shapeId), _position(std::move(position)), _size(std::move(size)), _transform(std::move(transform)), _impl(std::move(impl)), _disposed(std::move(disposed)), _callback(sp<Callback>::make())
{
}

RigidBody::Stub::~Stub()
{
    _disposed->dispose();
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
