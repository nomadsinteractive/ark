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

class DynamicPosition : public VV2 {
public:
    DynamicPosition(const sp<SimpleCollider::RigidBodyImpl>& rigidBody, const sp<VV>& position)
        : _rigid_body(rigidBody), _position(position), _bounds(0, 0, rigidBody->size()->width(), rigidBody->size()->height()) {
    }

    virtual V2 val() override {
        const sp<SimpleCollider::RigidBodyImpl> rigidBody = _rigid_body.lock();
        const V position = _position->val();
        if(rigidBody && rigidBody->id()) {
            _bounds.setCenter(position.x(), position.y());
            rigidBody->collision(_bounds);
            rigidBody->update();
        }
        return position;
    }

private:
    WeakPtr<SimpleCollider::RigidBodyImpl> _rigid_body;
    sp<VV> _position;
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
        rigidBody->setPosition(_resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(rigidBody, position)));
    }
    else if(type == Collider::BODY_TYPE_KINEMATIC)
    {
    }
    else if(type == Collider::BODY_TYPE_STATIC)
    {
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
    _x_axis_segment.remove(rigidBody.id());
    _y_axis_segment.remove(rigidBody.id());
    const auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", rigidBody.id());
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<SimpleCollider::RigidBodyImpl> SimpleCollider::Stub::createRigidBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const sp<SimpleCollider::Stub>& self)
{
    const sp<RigidBodyShadow> rigidBodyShadow = _object_pool.obtain<RigidBodyShadow>(++_rigid_body_base_id, type, shape, position->val(), size);
    const sp<RigidBodyImpl> rigidBody = _object_pool.obtain<RigidBodyImpl>(position, self, rigidBodyShadow);
    _rigid_bodies[rigidBody->id()] = rigidBodyShadow;
    insert(rigidBody);
    return rigidBody;
}

const sp<SimpleCollider::RigidBodyShadow>& SimpleCollider::Stub::ensureRigidBody(uint32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

const sp<SimpleCollider::RigidBodyShadow> SimpleCollider::Stub::findRigidBody(uint32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    return iter != _rigid_bodies.end() ? iter->second : sp<RigidBodyShadow>();
}

SimpleCollider::RigidBodyImpl::RigidBodyImpl(const sp<VV>& position, const sp<Stub>& collider, const sp<RigidBodyShadow>& shadow)
    : RigidBody(shadow->id(), shadow->type(), shadow->shape(), position, shadow->size(), Null::ptr<Numeric>()), _collider(collider), _shadow(shadow), _disposed(false)
{
}

SimpleCollider::RigidBodyImpl::~RigidBodyImpl()
{
    if(!_disposed)
        _collider->remove(*this);
}

void SimpleCollider::RigidBodyImpl::beginContact(const sp<RigidBody>& rigidBody)
{
    if(_shadow->collisionCallback())
        _shadow->collisionCallback()->onBeginContact(rigidBody);
}

void SimpleCollider::RigidBodyImpl::endContact(const sp<RigidBody>& rigidBody)
{
    if(_shadow->collisionCallback())
        _shadow->collisionCallback()->onEndContact(rigidBody);
}

const sp<CollisionCallback>& SimpleCollider::RigidBodyImpl::collisionCallback() const
{
    return _shadow->collisionCallback();
}

void SimpleCollider::RigidBodyImpl::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _shadow->setCollisionCallback(collisionCallback);
}

void SimpleCollider::RigidBodyImpl::dispose()
{
    DCHECK(!_disposed, "RigidBody has been disposed already.");
    _collider->remove(*this);
    _disposed = true;
}

void SimpleCollider::RigidBodyImpl::setPosition(const sp<VV>& position)
{
    _position = position;
}

void SimpleCollider::RigidBodyImpl::collision(const Rect& rect)
{
    std::set<uint32_t> candidates;
    std::set<uint32_t> contacts = _contacts;
    const std::set<uint32_t> x = _collider->_x_axis_segment.findCandidates(rect.left(), rect.right());
    if(x.size())
    {
        const std::set<uint32_t> y = _collider->_y_axis_segment.findCandidates(rect.top(), rect.bottom());
        std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), std::inserter(candidates, candidates.begin()));
        for(auto iter = candidates.begin(); iter != candidates.end();)
        {
            uint32_t id = *iter;
            if(id == _id)
            {
                iter = candidates.erase(iter);
                continue;
            }
            const sp<RigidBodyShadow>& rigidBody = _collider->ensureRigidBody(id);
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
            {
                const sp<RigidBodyShadow> s = _collider->findRigidBody(i);
                endContact(s ? s : _collider->_object_pool.obtain<RigidBodyShadow>(i, Collider::BODY_TYPE_DYNAMIC, Collider::BODY_SHAPE_BOX, V(), nullptr));
            }
        }
        _contacts = candidates;
    }
}

void SimpleCollider::RigidBodyImpl::update()
{
    const V pos = _position->val();
    _shadow->setPosition(pos);
    _collider->_x_axis_segment.update(_id, pos.x(), _size->width());
    _collider->_y_axis_segment.update(_id, pos.y(), _size->height());
}

SimpleCollider::RigidBodyShadow::RigidBodyShadow(uint32_t id, Collider::BodyType type, Collider::BodyShape shape, const V& pos, const sp<Size>& size)
    : RigidBody(id, type, shape, sp<VV::Impl>::make(pos), size, nullptr)
{
    _position = static_cast<sp<VV::Impl>>(position());
}

void SimpleCollider::RigidBodyShadow::dispose()
{
    FATAL("RigidBody may not be disposed");
}

const sp<CollisionCallback>& SimpleCollider::RigidBodyShadow::collisionCallback() const
{
    return _collision_callback;
}

void SimpleCollider::RigidBodyShadow::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _collision_callback = collisionCallback;
}

void SimpleCollider::RigidBodyShadow::setPosition(const V& pos)
{
    _position->set(pos);
}

}
