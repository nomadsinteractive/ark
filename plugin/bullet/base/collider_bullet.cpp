#include "bullet/base/collider_bullet.h"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/boolean_type.h"

#include "graphics/base/quaternion.h"
#include "graphics/base/size.h"
#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"
#include "app/traits//shape.h"

#include "bullet/base/bt_rigidbody_ref.h"
#include "bullet/base/collision_shape.h"
#include "bullet/base/rigidbody_bullet.h"

namespace ark::plugin::bullet {

namespace {

class DynamicPosition final : public Vec3 {
public:
    DynamicPosition(const sp<btMotionState>& motionState, bool isStaticBody)
        : _motion_state(motionState), _is_static_body(isStaticBody) {
    }

    bool update(uint64_t timestamp) override
    {
        return !_is_static_body;
    }

    V3 val() override
    {
        return getWorldPosition();
    }

private:
    V3 getWorldPosition() const
    {
        btTransform transform;
        _motion_state->getWorldTransform(transform);
        const btVector3& pos = transform.getOrigin();
        return {pos.x(), pos.y(), pos.z()};
    }

private:
    sp<btMotionState> _motion_state;
    bool _is_static_body;
};

class DynamicTransform final : public Transform::Delegate {
public:
    DynamicTransform(const sp<btMotionState>& motionState)
        : _motion_state(motionState) {
    }

    bool update(const Transform::Stub& transform, uint64_t timestamp) override
    {
        return true;
    }

    void snapshot(const Transform::Stub& /*transform*/, Transform::Snapshot& snapshot) const override
    {
        btTransform transform;
        _motion_state->getWorldTransform(transform);
        transform.setOrigin(btVector3(0, 0, 0));
        transform.getOpenGLMatrix(snapshot.makeData<M4>().value());
    }

    V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override
    {
        return position;
    }

    M4 toMatrix(const Transform::Snapshot& snapshot) const override
    {
        return snapshot.getData<M4>();
    }

private:
    sp<btMotionState> _motion_state;
};

}

ColliderBullet::ColliderBullet(const V3& gravity, sp<ModelLoader> modelLoader)
    : _stub(sp<Stub>::make(gravity, std::move(modelLoader)))
{
    _stub->_dynamics_world->setInternalTickCallback(myInternalPreTickCallback, this, true);
    _stub->_dynamics_world->setInternalTickCallback(myInternalTickCallback, this);
}

sp<Rigidbody> ColliderBullet::createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotation, sp<Boolean> discarded)
{
    btTransform transform;
    const V3 pos = position->val();
    const V4 quat = rotation ? rotation->val() : constants::QUATERNION_ONE;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));
    transform.setRotation(btQuaternion(quat.x(), quat.y(), quat.z(), quat.w()));
    DCHECK(!discarded, "Unimplemented");

    sp<CollisionShape> cs;
    if(const auto iter = _stub->_collision_shapes.find(shape->type().id()); iter == _stub->_collision_shapes.end())
    {
        btCollisionShape* btShape;
        const V3 size = shape->size().val();
        switch(shape->type().id())
        {
        case Shape::TYPE_BOX:
            btShape = new btBoxShape(btVector3(size.x() / 2, size.y() / 2, size.z() / 2));
            break;
        case Shape::TYPE_BALL:
            btShape = new btSphereShape(size.x() / 2);
            break;
        case Shape::TYPE_CAPSULE:
            CHECK_WARN(size.y() > size.x(), "When constructing a capsule shape, its height(%.2f) needs be greater than its width(%.2f)", size.y(), size.x());
            btShape = new btCapsuleShape(size.x() / 2, size.y() - size.x());
            break;
        default:
            FATAL("Undefined shape type %d(%s) in this world", shape->type().id(), shape->type().name().c_str());
            break;
        }
        cs = sp<CollisionShape>::make(btShape, 1.0f);
    }
    else
        cs = iter->second;

    if(type == BODY_TYPE_SENSOR)
    {
        sp<BtRigidbodyRef> rigidbody = makeGhostObject(btDynamicWorld(), cs->btShape(), type);
        _stub->_kinematic_objects.emplace_back(KinematicObject(position, rotation, rigidbody));
        return sp<RigidbodyBullet>::make(++ _stub->_body_id_base, type, *this, std::move(cs), position,
                                         sp<Transform>::make(Transform::TYPE_LINEAR_3D, rotation), std::move(rigidbody));
    }

    const float mass = type == BODY_TYPE_DYNAMIC ? cs->mass() : 0;
    sp<btMotionState> motionState = sp<btDefaultMotionState>::make(transform);
    sp<BtRigidbodyRef> rigidBody = makeRigidBody(btDynamicWorld(), cs->btShape(), motionState.get(), type, mass);
    return sp<RigidbodyBullet>::make(++ _stub->_body_id_base, type, *this, std::move(cs), sp<DynamicPosition>::make(motionState, type == BODY_TYPE_STATIC),
                                     sp<Transform>::make(sp<DynamicTransform>::make(motionState)), std::move(rigidBody));
}

void ColliderBullet::rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback, int32_t filterGroup, int32_t filterMask) const
{
    btVector3 btFrom(from.x(), from.y(), from.z());
    btVector3 btTo(to.x(), to.y(), to.z());
    btCollisionWorld::ClosestRayResultCallback closestResults(btFrom, btTo);
    closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
    closestResults.m_collisionFilterGroup = filterGroup;
    closestResults.m_collisionFilterMask = filterMask;

    _stub->_dynamics_world->rayTest(btVector3(from.x(), from.y(), from.z()), btVector3(to.x(), to.y(), to.z()), closestResults);
    if(closestResults.hasHit())
    {
        const btVector3 p = btFrom.lerp(btTo, closestResults.m_closestHitFraction);
        const btVector3& n = closestResults.m_hitNormalWorld;
        const CollisionManifold manifold(V3(p.x(), p.y(), p.z()), V3(n.x(), n.y(), n.z()));
        callback->onBeginContact(getRigidBodyFromCollisionObject(closestResults.m_collisionObject), manifold);
    }
}

std::vector<RayCastManifold> ColliderBullet::rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    btVector3 btFrom(from.x(), from.y(), from.z());
    btVector3 btTo(to.x(), to.y(), to.z());
    btCollisionWorld::AllHitsRayResultCallback allHitResults(btFrom, btTo);
    allHitResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

    _stub->_dynamics_world->rayTest(btVector3(from.x(), from.y(), from.z()), btVector3(to.x(), to.y(), to.z()), allHitResults);

    std::vector<RayCastManifold> manifolds;
    if(allHitResults.hasHit())
    {
        for(int32_t i = 0; i < allHitResults.m_hitFractions.size(); ++i)
        {
            const btVector3& n = allHitResults.m_hitNormalWorld.at(i);
            const RigidbodyBullet& rigidBody = getRigidBodyFromCollisionObject(allHitResults.m_collisionObjects.at(i));
            manifolds.emplace_back(allHitResults.m_hitFractions.at(i), V3(n.x(), n.y(), n.z()), rigidBody.ref());
        }
    }
    return manifolds;
}

void ColliderBullet::run()
{
    _stub->step();
}

btDiscreteDynamicsWorld* ColliderBullet::btDynamicWorld() const
{
    return _stub->_dynamics_world.get();
}

const std::unordered_map<TypeId, sp<CollisionShape>>& ColliderBullet::collisionShapes() const
{
    return _stub->_collision_shapes;
}

std::unordered_map<TypeId, sp<CollisionShape>>& ColliderBullet::collisionShapes()
{
    return _stub->_collision_shapes;
}

void ColliderBullet::myInternalPreTickCallback(btDynamicsWorld* dynamicsWorld, btScalar /*timeStep*/)
{
    ColliderBullet* self = static_cast<ColliderBullet*>(dynamicsWorld->getWorldUserInfo());
    const uint64_t tick = Ark::instance().applicationContext()->renderController()->tick();
    for(const KinematicObject& i : self->_stub->_kinematic_objects)
    {
        i._position.update(tick);
        i._quaternion.update(tick);
        V3 pos = i._position.val();
        const V4 quaternion = i._quaternion.val();
        btTransform transform;
        transform.setRotation(btQuaternion(quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w()));
        transform.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));
        i._rigid_body->collisionObject()->setWorldTransform(transform);
    }
}

void ColliderBullet::myInternalTickCallback(btDynamicsWorld* dynamicsWorld, btScalar /*timeStep*/)
{
    int32_t numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    ColliderBullet* self = reinterpret_cast<ColliderBullet*>(dynamicsWorld->getWorldUserInfo());
    for(int32_t i = 0; i < numManifolds; ++i)
    {
        btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

        if(contactManifold->getNumContacts() > 0)
        {
            const RigidbodyBullet& objA = getRigidBodyFromCollisionObject(contactManifold->getBody0());
            const sp<CollisionCallback>& ccObjA = objA.collisionCallback();
            const RigidbodyBullet& objB = getRigidBodyFromCollisionObject(contactManifold->getBody1());
            const sp<CollisionCallback>& ccObjB = objB.collisionCallback();

            const sp<BtRigidbodyRef>& refA = objA.rigidBody();
            const sp<BtRigidbodyRef>& refB = objB.rigidBody();

            if(ccObjA || ccObjB)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(0);
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& normalOnB = pt.m_normalWorldOnB;
                const V3 cp(ptA.x(), ptA.y(), ptA.z());
                const V3 normal(normalOnB.x(), normalOnB.y(), normalOnB.z());

                if(ccObjA)
                    self->addTickContactInfo(refA, ccObjA, refB, cp, normal);

                if(ccObjB)
                    self->addTickContactInfo(refB, ccObjB, refA, cp, -normal);
            }
        }
    }

    for(auto iter = self->_contact_infos.begin(); iter != self->_contact_infos.end(); ++iter)
    {
        const sp<BtRigidbodyRef>& rigidBody = iter->first;
        ContactInfo& contactInfo = iter->second;
        if(rigidBody->collisionObject() == nullptr)
            contactInfo._last_tick.clear();
        else
        {
            const RigidbodyBullet& obj = getRigidBodyFromCollisionObject(iter->first->collisionObject());
            for(const sp<BtRigidbodyRef>& i : contactInfo._last_tick)
                if(i->collisionObject())
                {
                    if(contactInfo._current_tick.find(i) == contactInfo._current_tick.end())
                        if(obj.collisionCallback())
                            obj.collisionCallback()->onEndContact(getRigidBodyFromCollisionObject(i->collisionObject()));
                }

            contactInfo._last_tick = std::move(contactInfo._current_tick);
        }
        if(contactInfo._last_tick.empty())
            if((iter = self->_contact_infos.erase(iter)) == self->_contact_infos.end())
                break;
    }
}

const RigidbodyBullet& ColliderBullet::getRigidBodyFromCollisionObject(const btCollisionObject* collisionObject)
{
    return *static_cast<RigidbodyBullet*>(collisionObject->getUserPointer());
}

void ColliderBullet::addTickContactInfo(const sp<BtRigidbodyRef>& rigidBody, const sp<CollisionCallback>& callback, const sp<BtRigidbodyRef>& contact, const V3& cp, const V3& normal)
{
    ContactInfo& contactInfo = _contact_infos[rigidBody];
    contactInfo._current_tick.insert(contact);
    if(contactInfo._last_tick.find(contact) == contactInfo._last_tick.end())
        callback->onBeginContact(getRigidBodyFromCollisionObject(contact->collisionObject()), CollisionManifold(cp, normal));
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

sp<BtRigidbodyRef> ColliderBullet::makeRigidBody(btDynamicsWorld* world, btCollisionShape* shape, btMotionState* motionState, Collider::BodyType bodyType, btScalar mass)
{
    DASSERT(bodyType == Collider::BODY_TYPE_STATIC || bodyType == Collider::BODY_TYPE_DYNAMIC || bodyType == Collider::BODY_TYPE_KINEMATIC);

    btVector3 localInertia(0, 0, 0);
    if(mass != 0.f)
        shape->calculateLocalInertia(mass, localInertia);

    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, localInertia);

    btRigidBody* rigidBody = new btRigidBody(cInfo);
    if(bodyType == Collider::BODY_TYPE_KINEMATIC)
    {
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
    }
    world->addRigidBody(rigidBody);
    rigidBody->setUserIndex(-1);
    return sp<BtRigidbodyRef>::make(rigidBody);
}

sp<BtRigidbodyRef> ColliderBullet::makeGhostObject(btDynamicsWorld* world, btCollisionShape* shape, Collider::BodyType bodyType)
{
    DASSERT(bodyType == Collider::BODY_TYPE_SENSOR);
    btGhostObject* ghostObject = new btGhostObject();
    ghostObject->setCollisionShape(shape);
    ghostObject->setCollisionFlags(ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    ghostObject->setUserIndex(-1);
    world->addCollisionObject(ghostObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
    return sp<BtRigidbodyRef>::make(ghostObject);
}

ColliderBullet::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _gravity(Documents::getAttribute<V3>(manifest, "gravity", {0, -9.8, 0})), _model_loader(factory.getBuilder<ModelLoader>(manifest, "model-loader")),
      _resource_loader_context(resourceLoaderContext)
{
    for(const auto& i : manifest->children("import"))
        _importers.emplace_back(factory.ensureBuilder<RigidBodyImporter>(i), i);
}

sp<ColliderBullet> ColliderBullet::BUILDER_IMPL1::build(const Scope& args)
{
    const sp<ColliderBullet> collider = sp<ColliderBullet>::make(_gravity, _model_loader->build(args));
    for(const auto& i : _importers)
        i.first->build(args)->import(collider, i.second);
    _resource_loader_context->renderController()->addPreComposeRunnable(collider, BooleanType::__or__(_resource_loader_context->disposed(), sp<Boolean>::make<BooleanByWeakRef<ColliderBullet>>(collider, 1)));
    return collider;
}

ColliderBullet::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Collider> ColliderBullet::BUILDER_IMPL2::build(const Scope& args)
{
    return _impl.build(args);
}

ColliderBullet::KinematicObject::KinematicObject(sp<Vec3> position, sp<Vec4> quaternion, sp<BtRigidbodyRef> rigidBody)
    : _position(std::move(position)), _quaternion(std::move(quaternion), constants::QUATERNION_ONE), _rigid_body(std::move(rigidBody))
{
}

FilterAction ColliderBullet::KinematicObject::ListFilter::operator() (const KinematicObject& item) const
{
    return item._rigid_body.unique() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
}

}
