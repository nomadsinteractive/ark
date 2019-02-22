#include "app/base/rigid_body.h"

#include <algorithm>
#include <iterator>

#include "core/ark.h"

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"

#include "graphics/base/size.h"
#include "graphics/base/v3.h"
#include "graphics/base/render_object.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"

namespace ark {

RigidBody::RigidBody(int32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Rotation>& rotate)
    : _stub(sp<Stub>::make(id, type, position, size, rotate))
{
}

RigidBody::RigidBody(const sp<RigidBody::Stub>& stub)
    : _stub(stub)
{
}

void RigidBody::bind(const sp<RenderObject>& renderObject)
{
    renderObject->setPosition(position());
    renderObject->transform()->setRotation(rotation());
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

const sp<Vec>& RigidBody::position() const
{
    return _stub->_position;
}

const sp<Size>& RigidBody::size() const
{
    return _stub->_size;
}

const sp<Rotation>& RigidBody::rotation() const
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

RigidBody::Stub::Stub(int32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Rotation>& rotate, const sp<Callback>& callback)
    : _id(id), _type(type), _position(position), _size(size), _rotate(rotate), _callback(callback ? callback : sp<Callback>::make())
{
}

void RigidBody::Callback::onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    if(_collision_callback)
    {
        const sp<CollisionCallback> cc = _collision_callback;
        const sp<RigidBody> rb = rigidBody;
        const CollisionManifold m = manifold;
        Ark::instance().applicationContext()->post([=]() {
            cc->onBeginContact(rb, m);
        });
    }
}

void RigidBody::Callback::onEndContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback)
    {
        const sp<CollisionCallback> cc = _collision_callback;
        const sp<RigidBody> rb = rigidBody;
        Ark::instance().applicationContext()->post([=]() {
            cc->onEndContact(rb);
        });
    }
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

sp<RenderObject> RigidBody::RIGID_BODY_STYLE::build(const sp<Scope>& args)
{
    const sp<RenderObject> renderObject = _delegate->build(args);
    const sp<RigidBody> rigidBody = _rigid_body->build(args);
    rigidBody->bind(renderObject);
    return renderObject;
}

}
