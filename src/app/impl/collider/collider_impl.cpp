#include "app/impl/collider/collider_impl.h"

#include <algorithm>
#include <iterator>

#include "core/types/null.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/base/rigid_body.h"
#include "app/inf/collision_callback.h"
#include "app/inf/tracker.h"

namespace ark {

namespace {

class DynamicPosition : public Vec2 {
public:
    DynamicPosition(const sp<ColliderImpl::Stub>& collider, const sp<Vec2>& position)
        : _collider(collider), _position(position) {
    }

    void setRigidBody(const sp<ColliderImpl::RigidBodyShadow>& rigidBody) {
        _rigid_body_shadow = rigidBody;
        _aabb = Rect(0, 0, _rigid_body_shadow->size()->width(), _rigid_body_shadow->size()->height());
    }

    virtual V2 val() override {
        const V2 position = _position->val();
        if(_rigid_body_shadow && _rigid_body_shadow->isDisposed())
            _rigid_body_shadow = nullptr;
        if(_rigid_body_shadow)
            _rigid_body_shadow->collision(_rigid_body_shadow, _collider, position, _aabb);
        return position;
    }

private:
    sp<ColliderImpl::Stub> _collider;
    sp<Vec2> _position;
    sp<ColliderImpl::RigidBodyShadow> _rigid_body_shadow;
    Rect _aabb;
};

}

ColliderImpl::ColliderImpl(const sp<Tracker>& tracker, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make(tracker, manifest)), _resource_loader_context(resourceLoaderContext)
{
}

ColliderImpl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _tracker(factory.ensureBuilder<Tracker>(manifest, "tracker"))
{
}

sp<Collider> ColliderImpl::BUILDER::build(const sp<Scope>& args)
{
    return sp<ColliderImpl>::make(_tracker->build(args), _manifest, _resource_loader_context);
}

sp<RigidBody> ColliderImpl::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate)
{
    DASSERT(position);
    DASSERT(size);
    const sp<Disposed> disposed = sp<Disposed>::make();
    if(type == Collider::BODY_TYPE_DYNAMIC)
    {
        const sp<DynamicPosition> dpos = sp<DynamicPosition>::make(_stub, position);
        const sp<Vec2> pos = _resource_loader_context->synchronize<V2>(dpos, disposed);
        const sp<RigidBodyImpl> rigidBody = _stub->createRigidBody(type, shape, pos, size, rotate, disposed, _stub);
        dpos->setRigidBody(rigidBody->shadow());
        return rigidBody;
    }
    DCHECK(type == Collider::BODY_TYPE_KINEMATIC || type == Collider::BODY_TYPE_STATIC, "Unknown BodyType: %d", type);
    return _stub->createRigidBody(type, shape, position, size, rotate, disposed, _stub);
}

ColliderImpl::Stub::Stub(const sp<Tracker>& tracker, const document& manifest)
    : _tracker(tracker), _rigid_body_base_id(0)/*, _axises(sp<Axises>::make())*/
{
    loadShapes(manifest);
}

void ColliderImpl::Stub::remove(const RigidBody& rigidBody)
{
    _tracker->remove(rigidBody.id());
    const auto iter = _rigid_bodies.find(rigidBody.id());
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(%d) not found", rigidBody.id());
    LOGD("Removing RigidBody(%d)", rigidBody.id());
    _rigid_bodies.erase(iter);
}

sp<ColliderImpl::RigidBodyImpl> ColliderImpl::Stub::createRigidBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate, const sp<Disposed>& disposed, const sp<ColliderImpl::Stub>& self)
{
    uint32_t rigidBodyId = ++_rigid_body_base_id;
    float s = std::max(size->width(), size->height());
    const sp<Vec> dp = _tracker->create(rigidBodyId, position, sp<Vec::Const>::make(V(s, s)));
    const sp<RigidBodyShadow> rigidBodyShadow = _object_pool.obtain<RigidBodyShadow>(rigidBodyId, type, dp, size, rotate, disposed);
    const sp<RigidBodyImpl> rigidBody = _object_pool.obtain<RigidBodyImpl>(self, rigidBodyShadow);

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

    _rigid_bodies[rigidBodyShadow->id()] = rigidBodyShadow;
    return rigidBody;
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::Stub::ensureRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    DCHECK(iter != _rigid_bodies.end(), "RigidBody(id = %d) does not exists", id);
    return iter->second;
}

const sp<ColliderImpl::RigidBodyShadow> ColliderImpl::Stub::findRigidBody(int32_t id) const
{
    const auto iter = _rigid_bodies.find(id);
    return iter != _rigid_bodies.end() ? iter->second : sp<RigidBodyShadow>();
}

void ColliderImpl::Stub::loadShapes(const document& manifest)
{
    for(const document& i : manifest->children("shape"))
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

ColliderImpl::RigidBodyImpl::RigidBodyImpl(const sp<ColliderImpl::Stub>& collider, const sp<RigidBodyShadow>& shadow)
    : RigidBody(shadow->stub()), _collider(collider), _shadow(shadow)
{
}

ColliderImpl::RigidBodyImpl::~RigidBodyImpl()
{
    dispose();
}

const sp<ColliderImpl::RigidBodyShadow>& ColliderImpl::RigidBodyImpl::shadow() const
{
    return _shadow;
}

void ColliderImpl::RigidBodyImpl::dispose()
{
    _shadow->dispose(_collider);
}

ColliderImpl::RigidBodyShadow::RigidBodyShadow(uint32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate, const sp<Disposed>& disposed)
    : RigidBody(id, type, position, size, rotate, disposed), _c2_rigid_body(position, rotate, type == Collider::BODY_TYPE_STATIC), _dispose_requested(false)
{
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
    _dispose_requested = true;
}

void ColliderImpl::RigidBodyShadow::collision(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V& position, const Rect& aabb)
{
    if(_dispose_requested)
    {
        dispose(collider);
        return;
    }

    std::unordered_set<int32_t> candidates = collider._tracker->search(position, V(aabb.width(), aabb.height()));
    if(candidates.size())
    {
        std::unordered_set<int32_t> contacts = _contacts;
        const Stub& shadowStub = stub();
        for(auto iter = candidates.begin(); iter != candidates.end();)
        {
            int32_t id = *iter;
            if(id == shadowStub._id)
            {
                iter = candidates.erase(iter);
                continue;
            }
            const sp<RigidBodyShadow>& rigidBody = collider.ensureRigidBody(id);
            c2Manifold manifold;
            _c2_rigid_body.collideManifold(rigidBody->_c2_rigid_body, &manifold);
            if(manifold.count > 0)
            {
                auto iter2 = contacts.find(id);
                if(iter2 == contacts.end())
                    shadowStub._callback->onBeginContact(self, rigidBody, CollisionManifold(V(manifold.normal.x, manifold.normal.y)));
                else
                    contacts.erase(iter2);
                ++iter;
            }
            else
                iter = candidates.erase(iter);
        }
        for(int32_t i : contacts)
        {
            if(candidates.find(i) == candidates.end())
            {
                const sp<RigidBodyShadow> s = collider.findRigidBody(i);
                shadowStub._callback->onEndContact(self, s ? s : collider._object_pool.obtain<RigidBodyShadow>(i, Collider::BODY_TYPE_DYNAMIC, nullptr, nullptr, nullptr, nullptr));
            }
        }
        _contacts = candidates;
    }
}

void ColliderImpl::RigidBodyShadow::dispose(ColliderImpl::Stub& stub)
{
    if(!isDisposed())
    {
        stub.remove(*this);
        disposed()->dispose();
    }
}

bool ColliderImpl::RigidBodyShadow::isDisposed() const
{
    return disposed()->val();
}

Rect ColliderImpl::RigidBodyShadow::makeRigidBodyAABB() const
{
    DCHECK(stub()->_size, "RigidBody must have size defined");
    Rect aabb(0, 0, stub()->_size->width(), stub()->_size->height());
    if(_c2_rigid_body.isStaticBody())
    {
        const V2 pos = stub()->_position->val();
        aabb.setCenter(pos.x(), pos.y());
    }
    else
        aabb.setCenter(0, 0);
    return aabb;
}

}
