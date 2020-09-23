#include "bullet/base/collider_bullet.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/boolean_util.h"

#include "graphics/base/quaternion.h"
#include "graphics/base/size.h"
#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"

#include "bullet/base/collision_shape.h"
#include "bullet/rigid_body/rigid_body_bullet.h"

namespace ark {
namespace plugin {
namespace bullet {

ColliderBullet::ColliderBullet(const V3& gravity, sp<ModelLoader> modelLoader)
    : _stub(sp<Stub>::make(gravity, std::move(modelLoader)))
{
    _stub->_dynamics_world->setInternalTickCallback(myInternalTickCallback);
}

sp<RigidBody> ColliderBullet::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate)
{
    btTransform transform;
    const V3 pos = position->val();
    const V4 quat = rotate ? rotate->val() : V4(0, 0, 0, 1.0f);
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));
    transform.setRotation(btQuaternion(quat.x(), quat.y(), quat.z(), quat.w()));

    sp<CollisionShape> cs;
    const auto iter = _stub->_collision_shapes.find(shape);
    if(iter != _stub->_collision_shapes.end())
    {
        cs = iter->second;
    }
    else
    {
        btCollisionShape* btShape;
        switch(shape)
        {
        case Collider::BODY_SHAPE_BOX:
            btShape = new btBoxShape(btVector3(size->width() / 2, size->height() / 2, size->depth() / 2));
            break;
        case Collider::BODY_SHAPE_BALL:
            btShape = new btSphereShape(size->width() / 2);
            break;
        case Collider::BODY_SHAPE_CAPSULE:
            DWARN(size->height() > size->width(), "When constructing a capsule shape, its height(%.2f) needs be greater than its width(%.2f)", size->height(), size->width());
            btShape = new btCapsuleShapeZ(size->width() / 2, size->height() - size->width());
            break;
        default:
            DFATAL("Undefined RigidBody(%d) in this world", shape);
            break;
        }

        cs = sp<CollisionShape>::make(btShape, 1.0f);
    }
    return sp<RigidBodyBullet>::make(++ _stub->_body_id_base, type, *this, std::move(cs), transform, type == Collider::BODY_TYPE_DYNAMIC ? cs->mass() : 0);
}

void ColliderBullet::rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback)
{
    btVector3 btFrom(from.x(), from.y(), from.z());
    btVector3 btTo(to.x(), to.y(), to.z());
    btCollisionWorld::ClosestRayResultCallback closestResults(btFrom, btTo);
    closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

    _stub->_dynamics_world->rayTest(btVector3(from.x(), from.y(), from.z()), btVector3(to.x(), to.y(), to.z()), closestResults);
    if (closestResults.hasHit())
    {
        btVector3 p = btFrom.lerp(btTo, closestResults.m_closestHitFraction);
        const btVector3& n = closestResults.m_hitNormalWorld;
        CollisionManifold manifold(V3(p.x(), p.y(), p.z()), V3(n.x(), n.y(), n.z()));
        sp<RigidBody::Stub>& stub = *reinterpret_cast<sp<RigidBody::Stub>*>(closestResults.m_collisionObject->getUserPointer());
        callback->onBeginContact(sp<RigidBodyBullet>::make(stub), manifold);
    }
}

void ColliderBullet::rayCastAllHit(const V3& from, const V3& to, const sp<CollisionCallback>& callback)
{
    btVector3 btFrom(from.x(), from.y(), from.z());
    btVector3 btTo(to.x(), to.y(), to.z());
    btCollisionWorld::AllHitsRayResultCallback allHitResults(btFrom, btTo);
    allHitResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

    _stub->_dynamics_world->rayTest(btVector3(from.x(), from.y(), from.z()), btVector3(to.x(), to.y(), to.z()), allHitResults);
    if (allHitResults.hasHit())
    {
        btVector3 p = btFrom.lerp(btTo, allHitResults.m_closestHitFraction);
        const btVector3& n = allHitResults.m_hitNormalWorld[0];
        CollisionManifold manifold(V3(p.x(), p.y(), p.z()), V3(n.x(), n.y(), n.z()));
        callback->onBeginContact(nullptr, manifold);
    }
}

void ColliderBullet::run()
{
    _stub->step();
}

btDiscreteDynamicsWorld* ColliderBullet::btDynamicWorld() const
{
    return _stub->_dynamics_world.get();
}

const std::unordered_map<int32_t, sp<CollisionShape>>& ColliderBullet::collisionShapes() const
{
    return _stub->_collision_shapes;
}

std::unordered_map<int32_t, sp<CollisionShape>>& ColliderBullet::collisionShapes()
{
    return _stub->_collision_shapes;
}

void ColliderBullet::myInternalTickCallback(btDynamicsWorld* dynamicsWorld, btScalar /*timeStep*/)
{
    int32_t numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for(int32_t i = 0; i < numManifolds; ++i)
    {
        btPersistentManifold *contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

        const RigidBodyBullet objA(getRigidBodyFromCollisionObject(contactManifold->getBody0()));
        const sp<CollisionCallback>& ccObjA = objA.collisionCallback();
        const RigidBodyBullet objB(getRigidBodyFromCollisionObject(contactManifold->getBody1()));
        const sp<CollisionCallback>& ccObjB = objB.collisionCallback();

        int32_t numContacts = contactManifold->getNumContacts();
        for(int32_t j = 0; j < numContacts; ++j)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            const btVector3& ptA = pt.getPositionWorldOnA();
            const btVector3& ptB = pt.getPositionWorldOnB();
            const btVector3& normalOnB = pt.m_normalWorldOnB;
        }
    }
}

RigidBodyBullet ColliderBullet::getRigidBodyFromCollisionObject(const btCollisionObject* collisionObject)
{
    return RigidBodyBullet(*reinterpret_cast<sp<RigidBody::Stub>*>(collisionObject->getUserPointer()));
}

ColliderBullet::Stub::Stub(const V3& gravity, sp<ModelLoader> modelLoader)
    : _model_loader(std::move(modelLoader)), _collision_configuration(new btDefaultCollisionConfiguration()), _collision_dispatcher(new btCollisionDispatcher(_collision_configuration)),
      _broadphase(new btDbvtBroadphase()), _solver(new btSequentialImpulseConstraintSolver()), _dynamics_world(new btDiscreteDynamicsWorld(_collision_dispatcher, _broadphase, _solver, _collision_configuration)),
      _body_id_base(0), _time_step(1 / 60.0f)
{
    _dynamics_world->setGravity(btVector3(gravity.x(), gravity.y(), gravity.z()));
}

ColliderBullet::Stub::~Stub()
{
    dispose();
}

void ColliderBullet::Stub::step()
{
    _dynamics_world->stepSimulation(_time_step);
}

void ColliderBullet::Stub::dispose()
{
    for(int32_t i = _dynamics_world->getNumConstraints() - 1; i >= 0; i--)
        _dynamics_world->removeConstraint(_dynamics_world->getConstraint(i));

    for(int32_t i = _dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = _dynamics_world->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
            delete body->getMotionState();
        _dynamics_world->removeCollisionObject(obj);
        delete obj;
    }
}

ColliderBullet::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _gravity(factory.ensureBuilder<Vec3>(manifest, "gravity")), _model_loader(factory.getBuilder<ModelLoader>(manifest, "model-loader")),
      _resource_loader_context(resourceLoaderContext)
{
    for(const auto& i : manifest->children("import"))
        _importers.push_back({factory.ensureBuilder<RigidBodyImporter>(i), i});
}

sp<ColliderBullet> ColliderBullet::BUILDER_IMPL1::build(const Scope& args)
{
    const sp<ColliderBullet> collider = sp<ColliderBullet>::make(_gravity->build(args)->val(), _model_loader->build(args));

    for(const auto& i : _importers)
        i.first->build(args)->import(collider, i.second);
    _resource_loader_context->renderController()->addPreRenderRunRequest(collider, BooleanUtil::__or__(_resource_loader_context->disposed(), sp<Boolean>::make<BooleanByWeakRef<ColliderBullet>>(collider, 1)));
    return collider;
}

ColliderBullet::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(factory, manifest, resourceLoaderContext)
{
}

sp<Collider> ColliderBullet::BUILDER_IMPL2::build(const Scope& args)
{
    return _delegate.build(args);
}

}
}
}
