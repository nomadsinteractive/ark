#include "app/impl/collider/partition_collider.h"

#include "core/base/bean_factory.h"
#include "core/types/null.h"
#include "core/types/weak_ptr.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/rigid_body.h"
#include "app/inf/collision_callback.h"
#include "app/inf/partition.h"

namespace ark {

namespace {

class DynamicPosition : public VV {
public:
    DynamicPosition(const sp<PartitionCollider::RigidBodyImpl>& rigidBody)
        : _body_stub(rigidBody->stub()), _id(rigidBody->id()), _bounds(0, 0, rigidBody->size()->width(), rigidBody->size()->height())
          , _search_radius(Math::hypot(_bounds.width(), _bounds.height())) {
    }

    virtual V val() override {
        const V position = _body_stub->_position->val();
        _bounds.setCenter(position.x(), position.y());
        _body_stub->collision(_id, _bounds, _search_radius);
        return position;
    }

private:
    sp<PartitionCollider::RigidBodyImpl::Stub> _body_stub;
    uint32_t _id;
    Rect _bounds;
    float _search_radius;
};

}

PartitionCollider::PartitionCollider(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Partition>& partition)
    : _stub(sp<Stub>::make(partition)), _resource_loader_context(resourceLoaderContext)
{
}

PartitionCollider::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _partition(factory.ensureBuilder<Partition>(manifest, "partition"))
{
}

sp<Collider> PartitionCollider::BUILDER::build(const sp<Scope>& args)
{
    return sp<PartitionCollider>::make(_resource_loader_context, _partition->build(args));
}

sp<RigidBody> PartitionCollider::createBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size)
{
    const sp<RigidBodyImpl> rigidBody = _stub->createRigidBody(type, shape, position, size, _stub);

    if(type == Collider::BODY_TYPE_DYNAMIC)
    {
        const sp<VV> position = _resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(rigidBody));
        rigidBody->setPosition(position);
    }
    else if(type == Collider::BODY_TYPE_KINEMATIC)
    {
        DFATAL("Kinematic body unsupported");
    }
    else if(type == Collider::BODY_TYPE_STATIC)
    {
    }
    return rigidBody;
}

PartitionCollider::Stub::Stub(const sp<Partition>& partition)
    : _partition(partition), _search_radius(0)
{
}

void PartitionCollider::Stub::remove(const RigidBodyImpl& rigidBody)
{
    _partition->removePoint(rigidBody.id());
    auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody not in collider");
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<PartitionCollider::RigidBodyImpl> PartitionCollider::Stub::createRigidBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const sp<PartitionCollider::Stub>& self)
{
    uint32_t id = _partition->addPoint(position);
    const sp<PartitionCollider::RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(id, type, shape, position, size, self);
    _rigid_bodies[id] = rigidBody;
    _search_radius = std::max(_search_radius, Math::hypot(size->width(), size->height()));
    return rigidBody;
}

const sp<PartitionCollider::RigidBodyImpl>& PartitionCollider::Stub::findRigidBody(uint32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

PartitionCollider::RigidBodyImpl::RigidBodyImpl(uint32_t id, Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const WeakPtr<PartitionCollider::Stub>& stub)
    : RigidBody(id, type, shape, position, size, Null::ptr<Numeric>()), _stub(sp<Stub>::make(stub, position)), _disposed(false)
{
}

PartitionCollider::RigidBodyImpl::~RigidBodyImpl()
{
    if(!_disposed)
    {
        const sp<PartitionCollider::Stub> stub = _stub->_collider_stub.lock();
        if(stub)
            stub->remove(*this);
    }
}

void PartitionCollider::RigidBodyImpl::Stub::beginContact(const sp<RigidBodyImpl>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onBeginContact(rigidBody);
}

void PartitionCollider::RigidBodyImpl::Stub::endContact(const sp<PartitionCollider::RigidBodyImpl>& rigidBody)
{
    if(_collision_callback)
        _collision_callback->onEndContact(rigidBody);
}

const sp<PartitionCollider::RigidBodyImpl::Stub>& PartitionCollider::RigidBodyImpl::stub() const
{
    return _stub;
}

const sp<CollisionCallback>& PartitionCollider::RigidBodyImpl::collisionCallback() const
{
    return _stub->_collision_callback;
}

void PartitionCollider::RigidBodyImpl::setCollisionCallback(const sp<CollisionCallback>& collisionCallback)
{
    _stub->_collision_callback = collisionCallback;
}

void PartitionCollider::RigidBodyImpl::dispose()
{
    DCHECK(!_disposed, "RigidBody(%d) has already been disposed", _id);
    const sp<PartitionCollider::Stub> s = _stub->_collider_stub.ensure();
    s->remove(*this);
    _disposed = true;
}

void PartitionCollider::RigidBodyImpl::setPosition(const sp<VV>& position)
{
    _position = position;
}

V PartitionCollider::RigidBodyImpl::xy() const
{
    return _stub->_position->val();
}

PartitionCollider::RigidBodyImpl::Stub::Stub(const WeakPtr<PartitionCollider::Stub>& colliderStub, const sp<VV>& position)
    : _collider_stub(colliderStub), _position(position)
{
}

void PartitionCollider::RigidBodyImpl::Stub::collision(uint32_t selfId, const Rect& rect, float searchRadius)
{
    const sp<PartitionCollider::Stub> s = _collider_stub.ensure();
    std::set<uint32_t> candidates = _contacts;
    std::set<uint32_t> contacts;
    float x = (rect.left() + rect.right()) / 2.0f;
    float y = (rect.top() + rect.bottom()) / 2.0f;
    s->_partition->updatePoint(selfId);
    const List<uint32_t> results = s->_partition->within(x, y, searchRadius + s->_search_radius);
    for(uint32_t id : results)
        if(id != selfId)
        {
            const sp<RigidBodyImpl>& rigidBody = s->findRigidBody(id);
            const V2 pos = rigidBody->xy();
            float halfWidth = rigidBody->width() / 2.0f;
            float halfHeight = rigidBody->height() / 2.0f;
            const Rect other(pos.x() - halfWidth, pos.y() - halfHeight, pos.x() + halfWidth, pos.y() + halfHeight);
            if(other.overlap(rect))
            {
                contacts.insert(id);
                candidates.insert(id);
            }
        }
    for(uint32_t id : candidates)
        if(contacts.find(id) != contacts.end())
        {
            if(_contacts.find(id) == _contacts.end())
                beginContact(s->findRigidBody(id));
        }
        else
            endContact(s->findRigidBody(id));
    _contacts = contacts;
}

}
