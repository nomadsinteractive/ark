#include "app/impl/collider/collider_impl.h"

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
    DynamicPosition(const sp<ColliderImpl::Stub>& collider, const sp<ColliderImpl::RigidBodyShadow>& rigidBodyShadow, const sp<VV>& position)
        : _collider(collider), _rigid_body_shadow(rigidBodyShadow), _position(position), _aabb(0, 0, _rigid_body_shadow->size()->width(), _rigid_body_shadow->size()->height()) {
    }

    virtual V2 val() override {
        const V position = _position->val();
        if(!_rigid_body_shadow->disposed()) {
            _aabb.setCenter(position.x(), position.y());
            _rigid_body_shadow->setPosition(position);
            _rigid_body_shadow->collision(_collider, _aabb);
            if(!_rigid_body_shadow->disposed())
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

ColliderImpl::ColliderImpl(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make(manifest)), _resource_loader_context(resourceLoaderContext)
{
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext)
{
}

sp<Collider> ColliderImpl::BUILDER::build(const sp<Scope>& args)
{
    return sp<ColliderImpl>::make(_manifest, _resource_loader_context);
}

sp<RigidBody> ColliderImpl::createBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform)
{
    const sp<RigidBodyImpl> rigidBody = _stub->createRigidBody(type, shape, position, size, transform, _stub);

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

ColliderImpl::Stub::Stub(const document& manifest)
    : _rigid_body_base_id(0), _axises(sp<Axises>::make())
{
    loadShapes(manifest);
}

void ColliderImpl::Stub::remove(const RigidBodyImpl& rigidBody)
{
    _axises->remove(rigidBody);
    const auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", rigidBody.id());
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<ColliderImpl::Stub>& self)
{
    const sp<RigidBodyShadow> rigidBodyShadow = _object_pool.obtain<RigidBodyShadow>(++_rigid_body_base_id, type, position->val(), size, transform);
    const sp<RigidBodyImpl> rigidBody = _object_pool.obtain<RigidBodyImpl>(position, self, rigidBodyShadow);

    DWARN(_c2_shapes.find(BODY_SHAPE_AABB) == _c2_shapes.end()
          && _c2_shapes.find(BODY_SHAPE_BALL) == _c2_shapes.end()
          && _c2_shapes.find(BODY_SHAPE_BOX) == _c2_shapes.end(), "Default shape being overrided");
    switch(shape)
    {
    case BODY_SHAPE_AABB:
        rigidBodyShadow->makeAABB();
        break;
    case BODY_SHAPE_BALL:
        rigidBodyShadow->makeBall();
        break;
    case BODY_SHAPE_BOX:
        rigidBodyShadow->makeBox();
        break;
    default:
        const auto iter = _c2_shapes.find(shape);
        DCHECK(iter != _c2_shapes.end(), "Unknow shape: %d", shape);
        rigidBodyShadow->makeShape(iter->second.first, iter->second.second);
    }

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

void ColliderImpl::Stub::loadShapes(const document& manifest)
{
    for(const document& i : manifest->children())
    {
        int32_t shapeId = Documents::ensureAttribute<int32_t>(i, "shape-id");
        const String& shapeType = Documents::ensureAttribute(i, "shape-type");
        C2_TYPE type;
        C2Shape shape;
        if(shapeType == "capsule")
        {
            type = C2_CAPSULE;
            shape.capsule.a.x = Documents::ensureAttribute<float>(i, "ax");
            shape.capsule.a.y = Documents::ensureAttribute<float>(i, "ay");
            shape.capsule.b.x = Documents::ensureAttribute<float>(i, "bx");
            shape.capsule.b.y = Documents::ensureAttribute<float>(i, "by");
            shape.capsule.r = Documents::ensureAttribute<float>(i, "r");
        }
        else if(shapeType == "poly")
        {
            int32_t c = 0;
            type = C2_POLY;
            for(const document& j : i->children())
            {
                DCHECK(c < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
                shape.poly.verts[c].x = Documents::ensureAttribute<float>(j, "x");
                shape.poly.verts[c].y = Documents::ensureAttribute<float>(j, "y");
                c++;
            }
            shape.poly.count = c;
            c2MakePoly(&shape.poly);
        }
        else
            DFATAL("Unknow shape type: %s", shapeType.c_str());
        _c2_shapes[shapeId] = std::make_pair(type, shape);
    }
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

ColliderImpl::RigidBodyShadow::RigidBodyShadow(uint32_t id, Collider::BodyType type, const V& pos, const sp<Size>& size, const sp<Transform>& transform)
    : RigidBody(id, type, sp<VV::Impl>::make(pos), size, nullptr), _position(static_cast<sp<VV::Impl>>(position())),
      _c2_rigid_body(_position, transform, type == Collider::BODY_TYPE_STATIC), _disposed(false)
{
    setPosition(pos);
}

void ColliderImpl::RigidBodyShadow::makeAABB()
{
    _c2_rigid_body.makeAABB(makeRigidBodyAABB());
}

void ColliderImpl::RigidBodyShadow::makeBall()
{
    const Rect aabb = makeRigidBodyAABB();
    float radius = std::min(aabb.width(), aabb.height());
    DCHECK(radius >= 0, "Radius must greater than 0");
    _c2_rigid_body.makeCircle(V2((aabb.left() + aabb.right()) / 2.0f, (aabb.top() + aabb.bottom()) / 2.0f), radius);
}

void ColliderImpl::RigidBodyShadow::makeBox()
{
    const Rect aabb = makeRigidBodyAABB();
    c2Poly box;
    box.count = 4;
    box.verts[0] = {aabb.left(), aabb.top()};
    box.verts[1] = {aabb.left(), aabb.bottom()};
    box.verts[2] = {aabb.right(), aabb.bottom()};
    box.verts[3] = {aabb.right(), aabb.top()};
    _c2_rigid_body.makePoly(box);
}

void ColliderImpl::RigidBodyShadow::makeShape(C2_TYPE type, const C2Shape& shape)
{
    _c2_rigid_body.makeShape(type, shape);
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
            int32_t overlap = _c2_rigid_body.collide(rigidBody->_c2_rigid_body);
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
                endContact(s ? s : collider._object_pool.obtain<RigidBodyShadow>(i, Collider::BODY_TYPE_DYNAMIC, V(), nullptr, nullptr));
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

Rect ColliderImpl::RigidBodyShadow::makeRigidBodyAABB() const
{
    DCHECK(_size, "RigidBody must have size defined");
    Rect aabb(0, 0, _size->width(), _size->height());
    if(_c2_rigid_body.isStaticBody())
    {
        const V2 pos = _position->val();
        aabb.setCenter(pos.x(), pos.y());
    }
    else
        aabb.setCenter(0, 0);
    return aabb;
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
