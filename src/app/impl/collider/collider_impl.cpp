#include "app/impl/collider/collider_impl.h"

#include <algorithm>
#include <iterator>

#define TINYC2_IMPLEMENTATION
#include <tinyc2.h>

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
    DynamicPosition(const sp<ColliderImpl::Stub>& collider, const sp<ColliderImpl::RigidBodyShadow>& rigidBodyShadow, const sp<VV>& position)
        : _collider(collider), _rigid_body_shadow(rigidBodyShadow), _position(position), _aabb(0, 0, _rigid_body_shadow->size()->width(), _rigid_body_shadow->size()->height()) {
    }

    virtual V2 val() override {
        const V position = _position->val();
        if(!_rigid_body_shadow->disposed()) {
            _aabb.setCenter(position.x(), position.y());
            _rigid_body_shadow->setPosition(position);
            _rigid_body_shadow->collision(_collider, _aabb);
            _collider->_axises->update(_rigid_body_shadow->id(), position, _aabb);
        }
        return position;
    }

private:
    sp<ColliderImpl::Stub> _collider;
    sp<ColliderImpl::RigidBodyShadow> _rigid_body_shadow;
    sp<VV> _position;
    Rect _aabb;
};

}

ColliderImpl::ColliderImpl(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make()), _resource_loader_context(resourceLoaderContext)
{
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<Collider> ColliderImpl::BUILDER::build(const sp<Scope>& args)
{
    return sp<ColliderImpl>::make(_resource_loader_context);
}

sp<RigidBody> ColliderImpl::createBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size)
{
    const sp<RigidBodyImpl> rigidBody = _stub->createRigidBody(type, position, size, _stub);

    if(type == Collider::BODY_TYPE_DYNAMIC)
    {
        rigidBody->setPosition(_resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(_stub, rigidBody->shadow(), position)));
    }
    else if(type == Collider::BODY_TYPE_KINEMATIC)
    {
    }
    else if(type == Collider::BODY_TYPE_STATIC)
    {
    }
    return rigidBody;
}

ColliderImpl::Stub::Stub()
    : _rigid_body_base_id(0), _axises(sp<Axises>::make())
{
}

void ColliderImpl::Stub::remove(const RigidBodyImpl& rigidBody)
{
    _axises->remove(rigidBody);
    const auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", rigidBody.id());
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<ColliderImpl::Stub>& self)
{
    const sp<RigidBodyShadow> rigidBodyShadow = _object_pool.obtain<RigidBodyShadow>(++_rigid_body_base_id, type, position->val(), size);
    const sp<RigidBodyImpl> rigidBody = _object_pool.obtain<RigidBodyImpl>(position, self, rigidBodyShadow);
    _rigid_bodies[rigidBody->id()] = rigidBodyShadow;
    _axises->insert(rigidBody);
    return rigidBody;
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::Stub::ensureRigidBody(uint32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

const sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::findRigidBody(uint32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    return iter != _rigid_bodies.end() ? iter->second : sp<RigidBodyShadow>();
}

ColliderImpl::RigidBodyImpl::RigidBodyImpl(const sp<VV>& position, const sp<Stub>& collider, const sp<RigidBodyShadow>& shadow)
    : RigidBody(shadow->id(), shadow->type(), position, shadow->size(), Null::ptr<Numeric>()), _collider(collider), _shadow(shadow)
{
}

ColliderImpl::RigidBodyImpl::~RigidBodyImpl()
{
    if(!_shadow->_disposed)
        _collider->remove(*this);
}

const sp<CollisionCallback>& ColliderImpl::RigidBodyImpl::collisionCallback() const
{
    return _shadow->collisionCallback();
}

void ColliderImpl::RigidBodyImpl::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _shadow->setCollisionCallback(collisionCallback);
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::RigidBodyImpl::shadow() const
{
    return _shadow;
}

void ColliderImpl::RigidBodyImpl::dispose()
{
    DCHECK(!_shadow->_disposed, "RigidBody has been disposed already.");
    _shadow->_disposed = true;
    _collider->remove(*this);
}

void ColliderImpl::RigidBodyImpl::setPosition(const sp<VV>& position)
{
    _position = position;
}

ColliderImpl::RigidBodyShadow::RigidBodyShadow(uint32_t id, Collider::BodyType type, const V& pos, const sp<Size>& size)
    : RigidBody(id, type, sp<VV::Impl>::make(pos), size, nullptr), _disposed(false)
{
    _position = static_cast<sp<VV::Impl>>(position());
}

void ColliderImpl::RigidBodyShadow::dispose()
{
    FATAL("RigidBody may not be disposed");
}

const sp<CollisionCallback>& ColliderImpl::RigidBodyShadow::collisionCallback() const
{
    return _collision_callback;
}

void ColliderImpl::RigidBodyShadow::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _collision_callback = collisionCallback;
}

void ColliderImpl::RigidBodyShadow::setPosition(const V& pos)
{
    _position->set(pos);
}

void ColliderImpl::RigidBodyShadow::collision(ColliderImpl::Stub& collider, const Rect& aabb)
{
    std::set<uint32_t> candidates = collider._axises->findCandidates(aabb);
    if(candidates.size())
    {
        std::set<uint32_t> contacts = _contacts;
        for(auto iter = candidates.begin(); iter != candidates.end();)
        {
            uint32_t id = *iter;
            if(id == _id)
            {
                iter = candidates.erase(iter);
                continue;
            }
            const sp<RigidBodyShadow>& rigidBody = collider.ensureRigidBody(id);
            const sp<Size>& size = rigidBody->size();
            const V position = rigidBody->xy();
            const Rect irect(position.x() - size->width() / 2, position.y() - size->height() / 2, position.x() + size->width() / 2, position.y() + size->height() / 2);
            bool overlap = irect.overlap(aabb);
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
                const sp<RigidBodyShadow> s = collider.findRigidBody(i);
                endContact(s ? s : collider._object_pool.obtain<RigidBodyShadow>(i, Collider::BODY_TYPE_DYNAMIC, V(), nullptr));
            }
        }
        _contacts = candidates;
    }
}

bool ColliderImpl::RigidBodyShadow::disposed() const
{
    return _disposed;
}

void ColliderImpl::RigidBodyShadow::beginContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onBeginContact(rigidBody);
}

void ColliderImpl::RigidBodyShadow::endContact(const sp<RigidBody>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onEndContact(rigidBody);
}

void ColliderImpl::Axises::insert(const RigidBody& rigidBody)
{
    const V position = rigidBody.position()->val();
    const sp<Size>& size = rigidBody.size();
    _x_axis_segment.insert(rigidBody.id(), position.x(), size->width() / 2);
    _y_axis_segment.insert(rigidBody.id(), position.y(), size->height() / 2);
}

void ColliderImpl::Axises::remove(const RigidBody& rigidBody)
{
    _x_axis_segment.remove(rigidBody.id());
    _y_axis_segment.remove(rigidBody.id());
}

void ColliderImpl::Axises::update(uint32_t id, const V2& position, const Rect& aabb)
{
    _x_axis_segment.update(id, position.x(), aabb.width());
    _y_axis_segment.update(id, position.y(), aabb.height());
}

std::set<uint32_t> ColliderImpl::Axises::findCandidates(const Rect& aabb) const
{
    std::set<uint32_t> candidates;
    const std::set<uint32_t> x = _x_axis_segment.findCandidates(aabb.left(), aabb.right());
    if(x.size())
    {
        const std::set<uint32_t> y = _y_axis_segment.findCandidates(aabb.top(), aabb.bottom());
        std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), std::inserter(candidates, candidates.begin()));
    }
    return candidates;
}

}
