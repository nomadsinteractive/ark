#include "app/base/rigid_body.h"

#include <algorithm>
#include <iterator>

#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"

#include "graphics/base/size.h"
#include "graphics/base/v3.h"

#include "app/inf/collision_callback.h"

namespace ark {

RigidBody::RigidBody(uint32_t id, Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Rotate>& rotate)
    : _stub(sp<Stub>::make(id, type, position, size, rotate))
{
}

RigidBody::RigidBody(const sp<RigidBody::Stub>& stub)
    : _stub(stub)
{
}

uint32_t RigidBody::id() const
{
    return _stub->_id;
}

Collider::BodyType RigidBody::type() const
{
    return _stub->_type;
}

V RigidBody::xy() const
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

const sp<VV>& RigidBody::position() const
{
    return _stub->_position;
}

const sp<Size>& RigidBody::size() const
{
    return _stub->_size;
}

const sp<Rotate>& RigidBody::rotate() const
{
    return _stub->_rotate;
}

const Box& RigidBody::tag() const
{
    return _stub->_tag;
}

void RigidBody::setTag(const Box& box) const
{
    _stub->_tag = box;
}

const sp<CollisionCallback>& RigidBody::collisionCallback() const
{
    return _stub->_collision_callback;
}

void RigidBody::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _stub->_collision_callback = collisionCallback;
}

const sp<RigidBody::Stub>& RigidBody::stub() const
{
    return _stub;
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

RigidBody::Stub::Stub(uint32_t id, Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Rotate>& rotate)
    : _id(id), _type(type), _position(position), _size(size), _rotate(rotate)
{
}

void RigidBody::Stub::beginContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onBeginContact(rigidBody);
}

void RigidBody::Stub::endContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onEndContact(rigidBody);
}

void RigidBody::Stub::beginContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody)
{
    beginContact(rigidBody);
    rigidBody->stub()->beginContact(self);
}

void RigidBody::Stub::endContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody)
{
    endContact(rigidBody);
    rigidBody->stub()->endContact(self);
}

}
