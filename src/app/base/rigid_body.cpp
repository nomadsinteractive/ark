#include "app/base/rigid_body.h"

#include <algorithm>
#include <iterator>

#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"

#include "graphics/base/size.h"

#include "app/inf/collision_callback.h"

namespace ark {

RigidBody::RigidBody(uint32_t id, Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Numeric>& rotation)
    : _id(id), _type(type), _position(position), _size(size), _rotation(rotation)
{
}

uint32_t RigidBody::id() const
{
    return _id;
}

Collider::BodyType RigidBody::type() const
{
    return _type;
}

V2 RigidBody::xy() const
{
    const V pos =_position->val();
    return V2(pos.x(), pos.y());
}

float RigidBody::width() const
{
    return _size->width();
}

float RigidBody::height() const
{
    return _size->height();
}

const sp<VV>& RigidBody::position() const
{
    return _position;
}

const sp<Size>& RigidBody::size() const
{
    return _size;
}

const sp<Numeric>& RigidBody::rotation() const
{
    return _rotation;
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

}
