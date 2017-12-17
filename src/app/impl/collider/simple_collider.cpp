#include "app/impl/collider/simple_collider.h"

#include <algorithm>
#include <iterator>

#include "core/types/null.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/rigid_body.h"
#include "app/inf/collision_callback.h"

namespace ark {

namespace {

class DynamicPosition : public VV {
public:
    DynamicPosition(const sp<SimpleCollider::RigidBodyImpl>& rigidBody)
        : _body_stub(rigidBody->stub()), _bounds(0, 0, rigidBody->size()->width(), rigidBody->size()->height()) {
    }

    virtual V val() override {
        const V position = _body_stub->_position->val();
        _bounds.setCenter(position.x(), position.y());
        _body_stub->collision(_bounds);
        return position;
    }

private:
    sp<SimpleCollider::RigidBodyImpl::Stub> _body_stub;
    Rect _bounds;
};

}

SimpleCollider::SimpleCollider(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make()), _resource_loader_context(resourceLoaderContext)
{
}

SimpleCollider::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<Collider> SimpleCollider::BUILDER::build(const sp<Scope>& args)
{
    return sp<SimpleCollider>::make(_resource_loader_context);
}

sp<RigidBody> SimpleCollider::createBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size)
{
    const sp<RigidBodyImpl> rigidBody = _stub->createRigidBody(type, shape, position, size, _stub);

    if(type == Collider::BODY_TYPE_DYNAMIC)
    {
        const sp<VV> position = _resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(rigidBody));
        rigidBody->setPosition(position);
    }
    else if(type == Collider::BODY_TYPE_KINEMATIC)
    {
    }
    else if(type == Collider::BODY_TYPE_STATIC)
    {
        _stub->insert(rigidBody);
    }
    return rigidBody;
}

SimpleCollider::Stub::Stub()
    : _rigid_body_base_id(0)
{
}

void SimpleCollider::Stub::insert(const sp<RigidBodyImpl>& rigidObject)
{
    const V position = rigidObject->position()->val();
    const sp<Size>& size = rigidObject->size();
    _x_axis_segment.insert(rigidObject->id(), position.x(), size->width() / 2);
    _y_axis_segment.insert(rigidObject->id(), position.y(), size->height() / 2);
}

void SimpleCollider::Stub::remove(const RigidBodyImpl& rigidBody)
{
    if(rigidBody.type() == Collider::BODY_TYPE_STATIC)
    {
        _x_axis_segment.remove(rigidBody.id());
        _y_axis_segment.remove(rigidBody.id());
    }
    auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody not in collider");
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<SimpleCollider::RigidBodyImpl> SimpleCollider::Stub::createRigidBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const sp<SimpleCollider::Stub>& self)
{
    const sp<SimpleCollider::RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(++_rigid_body_base_id, type, shape, position, size, self);
    _rigid_bodies[rigidBody->id()] = rigidBody;
    return rigidBody;
}

sp<SimpleCollider::RigidBodyImpl> SimpleCollider::Stub::findRigidBody(uint32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

SimpleCollider::RigidBodyImpl::RigidBodyImpl(uint32_t id, Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const WeakPtr<SimpleCollider::Stub>& stub)
    : RigidBody(id, type, shape, position, size, Null::ptr<Numeric>()), _stub(sp<Stub>::make(stub, position))
{
}

SimpleCollider::RigidBodyImpl::~RigidBodyImpl()
{
    if(_id > 0)
    {
        const sp<SimpleCollider::Stub> stub = _stub->_collider_stub.lock();
        if(stub)
            stub->remove(*this);
    }
}

void SimpleCollider::RigidBodyImpl::Stub::beginContact(const sp<RigidBodyImpl>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onBeginContact(rigidBody);
}

void SimpleCollider::RigidBodyImpl::Stub::endContact(const sp<SimpleCollider::RigidBodyImpl>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onEndContact(rigidBody);
}

const sp<SimpleCollider::RigidBodyImpl::Stub>& SimpleCollider::RigidBodyImpl::stub() const
{
    return _stub;
}

const sp<CollisionCallback>& SimpleCollider::RigidBodyImpl::collisionCallback() const
{
    return _stub->_collision_callback;
}

void SimpleCollider::RigidBodyImpl::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _stub->_collision_callback = collisionCallback;
}

void SimpleCollider::RigidBodyImpl::dispose()
{
    const sp<SimpleCollider::Stub> s = _stub->_collider_stub.ensure();
    s->remove(*this);
    _id = 0;
}

void SimpleCollider::RigidBodyImpl::setPosition(const sp<VV>& position)
{
    _position = position;
}

V SimpleCollider::RigidBodyImpl::xy() const
{
    return _stub->_position->val();
}

SimpleCollider::RigidBodyImpl::Stub::Stub(const WeakPtr<SimpleCollider::Stub>& colliderStub, const sp<VV>& position)
    : _collider_stub(colliderStub), _position(position)
{
}

void SimpleCollider::RigidBodyImpl::Stub::collision(const Rect& rect)
{
    const sp<SimpleCollider::Stub> s = _collider_stub.ensure();
    std::set<uint32_t> candidates;
    std::set<uint32_t> contacts = _contacts;
    const std::set<uint32_t> x = s->_x_axis_segment.findCandidates(rect.left(), rect.right());
    if(x.size())
    {
        const std::set<uint32_t> y = s->_y_axis_segment.findCandidates(rect.top(), rect.bottom());
        std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), std::inserter(candidates, candidates.begin()));
        for(auto iter = candidates.begin(); iter != candidates.end();)
        {
            uint32_t id = *iter;
            const sp<RigidBodyImpl> rigidBody = s->findRigidBody(id);
            const sp<Size>& size = rigidBody->size();
            const V position = rigidBody->xy();
            const Rect irect(position.x() - size->width() / 2, position.y() - size->height() / 2, position.x() + size->width() / 2, position.y() + size->height() / 2);
            bool overlap = irect.overlap(rect);
            if(overlap)
            {
                auto iter2 = contacts.find(id);
                if(iter2 == contacts.end())
                    beginContact(rigidBody);
                else
                    contacts.erase(iter2);
                ++iter;
            }
            else
                iter = candidates.erase(iter);
        }
        for(uint32_t i : contacts)
        {
            if(candidates.find(i) == candidates.end())
                endContact(s->findRigidBody(i));
        }
        _contacts = candidates;
    }
}

}
