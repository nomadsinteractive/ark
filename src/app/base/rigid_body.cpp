#include "app/base/rigid_body.h"

#include <algorithm>
#include <iterator>

#include "core/ark.h"

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/holder_util.h"

#include "graphics/base/size.h"
#include "graphics/base/v3.h"
#include "graphics/base/render_object.h"
#include "graphics/util/vec3_type.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"

namespace ark {

RigidBody::RigidBody(int32_t id, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate, const sp<Disposed>& disposed)
    : _stub(sp<Stub>::make(id, type, position, size, rotate, disposed))
{
}

RigidBody::RigidBody(const sp<RigidBody::Stub>& stub)
    : _stub(stub)
{
}

void RigidBody::bind(const sp<RenderObject>& renderObject)
{
    renderObject->setPosition(position());
    renderObject->setTransform(_stub->_transform);
    _stub->_render_object = renderObject;
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
    return _stub->_size->width();
}

float RigidBody::height() const
{
    return _stub->_size->height();
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
    *(_stub->_tag) = box;
}

sp<RenderObject> RigidBody::renderObject() const
{
    return _stub->_render_object.lock();
}

const sp<Disposed>& RigidBody::disposed() const
{
    return _stub->_disposed;
}

const sp<CollisionCallback>& RigidBody::collisionCallback() const
{
    return _stub->_callback->_collision_callback;
}

void RigidBody::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _stub->_callback->_collision_callback = collisionCallback;
}

const sp<RigidBody::Stub>& RigidBody::stub() const
{
    return _stub;
}

const sp<RigidBody::Callback>& RigidBody::callback() const
{
    return _stub->_callback;
}

template<> ARK_API Collider::BodyType Conversions::to<String, Collider::BodyType>(const String& str)
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

RigidBody::Stub::Stub(int32_t id, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate, const sp<Disposed>& disposed, const sp<Callback>& callback, const sp<Box>& tag)
    : _id(id), _type(type), _position(position), _size(size), _transform(sp<Transform>::make(Transform::TYPE_LINEAR_3D, rotate)), _disposed(disposed), _callback(callback ? callback : sp<Callback>::make()), _tag(tag ? tag : sp<Box>::make())
{
}

RigidBody::Stub::~Stub()
{
    _disposed->dispose();
}

void RigidBody::Callback::onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    if(_collision_callback)
        _collision_callback->onBeginContact(rigidBody, manifold);
}

void RigidBody::Callback::onEndContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onEndContact(rigidBody);
}

void RigidBody::Callback::onBeginContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    onBeginContact(rigidBody, manifold);
    rigidBody->callback()->onBeginContact(self, CollisionManifold(-manifold.normal()));
}

void RigidBody::Callback::onEndContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody)
{
    onEndContact(rigidBody);
    rigidBody->callback()->onEndContact(self);
}

bool RigidBody::Callback::hasCallback() const
{
    return static_cast<bool>(_collision_callback);
}

RigidBody::RIGID_BODY_STYLE::RIGID_BODY_STYLE(BeanFactory& factory, const sp<Builder<RenderObject>>& delegate, const String& value)
    : _delegate(delegate), _rigid_body(factory.ensureBuilder<RigidBody>(value))
{
}

sp<RenderObject> RigidBody::RIGID_BODY_STYLE::build(const Scope& args)
{
    const sp<RenderObject> renderObject = _delegate->build(args);
    const sp<RigidBody> rigidBody = _rigid_body->build(args);
    rigidBody->bind(renderObject);
    return renderObject;
}

}
