#include "bullet/base/collider_bullet.h"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/boolean_type.h"

#include "graphics/base/v3.h"
#include "graphics/components/shape.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/collision_filter.h"
#include "app/base/collision_manifold.h"
#include "app/inf/collision_callback.h"

#include "bullet/base/collision_object_ref.h"
#include "bullet/base/collision_shape_ref.h"
#include "bullet/base/rigidbody_bullet.h"

namespace ark::plugin::bullet {

namespace {

class DynamicPosition final : public Vec3 {
public:
    DynamicPosition(sp<btMotionState> motionState, const V3 origin)
        : _motion_state(std::move(motionState)), _origin(origin) {
    }

    bool update(uint64_t timestamp) override
    {
        return true;
    }

    V3 val() override
    {
        return getWorldPosition() + _origin;
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
    V3 _origin;
};

class DynamicRotation final : public Vec4 {
public:
    DynamicRotation(sp<btMotionState> motionState)
        : _motion_state(std::move(motionState)) {
    }

    bool update(uint64_t timestamp) override
    {
        return true;
    }

    V4 val() override
    {
        btTransform transform;
        _motion_state->getWorldTransform(transform);
        const btQuaternion quaternion = transform.getRotation();
        return {quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w()};
    }

private:
    sp<btMotionState> _motion_state;
};

sp<CollisionShapeRef> makeConvexHullCollisionShape(const Model& model, btScalar mass)
{
    CHECK(!model.meshes().empty(), "ConvexHullRigidBodyImporter only works with Mesh based models");

    sp<btConvexHullShape> convexHullShape = sp<btConvexHullShape>::make();
    for(const Vector<Node::WithTransform> flattened = model.toFlattened<Node::WithTransform>(); const Node::WithTransform& i : flattened)
        for(const Mesh& j : i._node->meshes())
            for(const V3 k : j.vertices())
            {
                const V3 pt = MatrixUtil::transform(i._transform, k);
                convexHullShape->addPoint(btVector3(pt.x(), pt.y(), pt.z()), false);
            }

    convexHullShape->recalcLocalAabb();
    convexHullShape->optimizeConvexHull();
    return sp<CollisionShapeRef>::make(std::move(convexHullShape), mass);
}

sp<CollisionObjectRef> makeRigidBody(btDynamicsWorld* world, sp<CollisionShapeRef> shape, sp<btMotionState> motionState, const Rigidbody::BodyType bodyType, const btScalar mass, const sp<CollisionFilter>& collisionFilter)
{
    DASSERT(bodyType == Rigidbody::BODY_TYPE_STATIC || bodyType == Rigidbody::BODY_TYPE_DYNAMIC || bodyType == Rigidbody::BODY_TYPE_KINEMATIC);

    btVector3 localInertia(0, 0, 0);
    btCollisionShape* btShape = shape->btShape();
    if(mass != 0.f)
        btShape->calculateLocalInertia(mass, localInertia);

    const btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState.get(), btShape, localInertia);

    btRigidBody* rigidBody = new btRigidBody(cInfo);
    if(bodyType == Rigidbody::BODY_TYPE_KINEMATIC)
    {
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
    }
    if(collisionFilter)
        world->addRigidBody(rigidBody, collisionFilter->categoryBits(), collisionFilter->maskBits());
    else
        world->addRigidBody(rigidBody);

    return sp<CollisionObjectRef>::make(rigidBody, std::move(shape), std::move(motionState));
}

sp<CollisionObjectRef> makeGhostObject(btDynamicsWorld* world, sp<CollisionShapeRef> shape, Rigidbody::BodyType bodyType, const sp<CollisionFilter>& collisionFilter)
{
    btGhostObject* ghostObject = new btGhostObject();
    ghostObject->setCollisionShape(shape->btShape());
    ghostObject->setCollisionFlags(ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    ghostObject->setUserIndex(-1);
    if(collisionFilter)
        world->addCollisionObject(ghostObject, collisionFilter->categoryBits(), collisionFilter->maskBits());
    else
        world->addCollisionObject(ghostObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
    return sp<CollisionObjectRef>::make(ghostObject, std::move(shape));
}

RigidbodyBullet getRigidBodyFromCollisionObject(const btCollisionObject* collisionObject)
{
    return RigidbodyBullet::fromCollisionObjectPointer(collisionObject->getUserPointer());
}

struct BtRigibodyObject {
    BtRigibodyObject(RigidbodyBullet btRigidbodyRef)
        : _bt_rigidbody_ref(std::move(btRigidbodyRef))
    {
    }

    RigidbodyBullet _bt_rigidbody_ref;

    class ListFilter {
    public:
        ListFilter() = default;

        FilterAction operator() (const BtRigibodyObject& item) const
        {
            const btCollisionObject* collisionObject = item._bt_rigidbody_ref.collisionObjectRef()->collisionObject();
            if(!collisionObject || collisionObject->getUserPointer() == nullptr)
                return FILTER_ACTION_REMOVE;
            if(!item._bt_rigidbody_ref.validate())
                return FILTER_ACTION_REMOVE;
            return item._bt_rigidbody_ref.unique() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
        }
    };
};

struct GhostObject : BtRigibodyObject {
    GhostObject(sp<Vec3> position, sp<Vec4> quaternion, RigidbodyBullet btRigidbodyRef)
        : BtRigibodyObject(std::move(btRigidbodyRef)), _position(std::move(position)), _quaternion(std::move(quaternion), constants::QUATERNION_ONE) {
    }

    OptionalVar<Vec3> _position;
    OptionalVar<Vec4> _quaternion;
};

}

struct ColliderBullet::Stub final : Updatable {
    Stub(const V3 gravity, sp<ModelLoader> modelLoader)
        : _model_loader(std::move(modelLoader)), _collision_configuration(new btDefaultCollisionConfiguration()), _collision_dispatcher(new btCollisionDispatcher(_collision_configuration)),
          _broadphase(new btDbvtBroadphase()), _solver(new btSequentialImpulseConstraintSolver()), _dynamics_world(new btDiscreteDynamicsWorld(_collision_dispatcher, _broadphase, _solver, _collision_configuration)),
          _app_clock_interval(Ark::instance().applicationContext()->appClockInterval())
    {
        _dynamics_world->setGravity(btVector3(gravity.x(), gravity.y(), gravity.z()));
    }

    ~Stub() override
    {
        discard();
    }

    bool update(const uint64_t timestamp) override
    {
        _timestamp = timestamp;
        _dynamics_world->stepSimulation(_app_clock_interval->val());
        return true;
    }

    void discard() const
    {
        for(int32_t i = _dynamics_world->getNumConstraints() - 1; i >= 0; i--)
            _dynamics_world->removeConstraint(_dynamics_world->getConstraint(i));

        btCollisionObjectArray& coArray = _dynamics_world->getCollisionObjectArray();
        for(int32_t i = _dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = coArray[i];
            if(btRigidBody* body = btRigidBody::upcast(obj); body && body->getMotionState())
                delete body->getMotionState();
            _dynamics_world->removeCollisionObject(obj);
            delete obj;
        }
    }

    sp<ModelLoader> _model_loader;
    HashMap<TypeId, sp<CollisionShapeRef>> _collision_shapes;

    op<btDefaultCollisionConfiguration> _collision_configuration;
    op<btCollisionDispatcher> _collision_dispatcher;

    op<btBroadphaseInterface> _broadphase;
    op<btConstraintSolver> _solver;
    op<btDiscreteDynamicsWorld> _dynamics_world;

    FList<GhostObject, GhostObject::ListFilter> _ghost_objects;
    Vector<sp<CollisionObjectRef>> _mark_for_destroys;

    sp<Numeric> _app_clock_interval;
    uint64_t _timestamp;
};

ColliderBullet::ColliderBullet(const V3 gravity, sp<ModelLoader> modelLoader)
    : _stub(sp<Stub>::make(gravity, std::move(modelLoader)))
{
    _stub->_dynamics_world->setInternalTickCallback(myInternalPreTickCallback, this, true);
    _stub->_dynamics_world->setInternalTickCallback(myInternalTickCallback, this);
}

Rigidbody::Impl ColliderBullet::createBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    sp<CollisionShapeRef> cs = shape->asImplementation<CollisionShapeRef>();
    if(!cs)
    {
        if(const auto iter = _stub->_collision_shapes.find(shape->type().hash()); iter == _stub->_collision_shapes.end())
        {
            btCollisionShape* btShape = nullptr;
            const V3 size = shape->size().val();
            switch(shape->type().hash())
            {
            case Shape::TYPE_AABB:
            case Shape::TYPE_BOX:
                btShape = new btBoxShape(btVector3(size.x() / 2, size.y() / 2, size.z() / 2));
                break;
            case Shape::TYPE_BALL:
                btShape = new btSphereShape(size.x() / 2);
                break;
            case Shape::TYPE_CAPSULE:
                {
                    const float diameter = std::min(size.x(), size.z());
                    ASSERT(diameter > 0);
                    const float radius = diameter / 2.0f;
                    const float height = size.y() - diameter;
                    CHECK_WARN(height >= 0, "When constructing a btCapsuleShapeY, its height(%.2f) should be greater than its diameter(%.2f)", size.y(), diameter);
                    btShape = new btCapsuleShape(radius, std::max(height, 0.0f));
                    break;
                }
            default:
                FATAL("Undefined shape type %d(%s) in this world", shape->type().hash(), shape->type().name().c_str());
                break;
            }
            cs = sp<CollisionShapeRef>::make(sp<btCollisionShape>::adopt(btShape), 1.0f);
        }
        else
            cs = iter->second;
    }

    if(type == Rigidbody::BODY_TYPE_SENSOR || type == Rigidbody::BODY_TYPE_GHOST)
    {
        sp<Vec3> originPosition = shape->origin() ? Vec3Type::add(position, shape->origin().val()) : position;
        sp<CollisionObjectRef> btGhostObjectRef = makeGhostObject(btDynamicWorld(), std::move(cs), type, collisionFilter);
        sp<RigidbodyBullet> impl = sp<RigidbodyBullet>::make(*this, std::move(btGhostObjectRef), type, std::move(shape), std::move(position), rotation, std::move(collisionFilter), std::move(discarded));
        _stub->_ghost_objects.emplace_back(GhostObject(std::move(originPosition), std::move(rotation), *impl));
        sp<Rigidbody::Stub> stub = impl->stub();
        return {std::move(stub), nullptr, impl};
    }

    btTransform btTrans;
    const V3 origin = shape->origin().val();
    const V3 pos = position->val() + origin;
    const V4 quat = rotation ? rotation->val() : constants::QUATERNION_ONE;
    btTrans.setIdentity();
    btTrans.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));
    btTrans.setRotation(btQuaternion(quat.x(), quat.y(), quat.z(), quat.w()));

    const float mass = type == Rigidbody::BODY_TYPE_DYNAMIC ? cs->mass() : 0;
    sp<btMotionState> motionState = sp<btDefaultMotionState>::make(btTrans);
    sp<CollisionObjectRef> btRigidbodyRef = makeRigidBody(btDynamicWorld(), std::move(cs), motionState, type, mass, collisionFilter);
    sp<Vec3> btPosition = type == Rigidbody::BODY_TYPE_STATIC ? sp<Vec3>::make<Vec3::Const>(pos - origin) : sp<Vec3>::make<DynamicPosition>(motionState, origin);
    sp<Vec4> btRotation = type == Rigidbody::BODY_TYPE_STATIC ? sp<Vec4>::make<Vec4::Const>(quat) : sp<Vec4>::make<DynamicRotation>(std::move(motionState));
    sp<RigidbodyBullet> impl = sp<RigidbodyBullet>::make(*this, std::move(btRigidbodyRef), type, std::move(shape), std::move(btPosition), std::move(btRotation), std::move(collisionFilter), std::move(discarded));
    sp<Rigidbody::Stub> stub = impl->stub();
    return {std::move(stub), nullptr, impl};
}

sp<Shape> ColliderBullet::createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
{
    const HashId shapeType = type.hash();
    if(shapeType == Shape::TYPE_AABB || shapeType == Shape::TYPE_BOX || shapeType == Shape::TYPE_BALL || shapeType == Shape::TYPE_CAPSULE)
        return sp<Shape>::make(type, std::move(size), std::move(origin));

    const sp<Model> model = _stub->_model_loader->loadModel(shapeType);
    CHECK(model, "Failed to load model[%ud(\"%s\")]", type.hash(), type.name().c_str());
    sp<Vec3> contentSize = size ? std::move(size) : sp<Vec3>(model->content()->size());
    const V3 contentSizeValue = contentSize->val();
    sp<CollisionShapeRef> collisionShape = makeConvexHullCollisionShape(model, contentSizeValue.x() * contentSizeValue.y() * contentSizeValue.z());
    return sp<Shape>::make(type, std::move(contentSize), std::move(origin), Box(std::move(collisionShape)));
}

void ColliderBullet::rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback, int32_t filterGroup, int32_t filterMask) const
{
    const btVector3 btFrom(from.x(), from.y(), from.z());
    const btVector3 btTo(to.x(), to.y(), to.z());
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
        callback->onBeginContact(getRigidBodyFromCollisionObject(closestResults.m_collisionObject).makeShadow(), manifold);
    }
}

Vector<RayCastManifold> ColliderBullet::rayCast(V3 from, V3 to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    const btVector3 btFrom(from.x(), from.y(), from.z());
    const btVector3 btTo(to.x(), to.y(), to.z());
    btCollisionWorld::AllHitsRayResultCallback allHitResults(btFrom, btTo);
    allHitResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

    _stub->_dynamics_world->rayTest(btVector3(from.x(), from.y(), from.z()), btVector3(to.x(), to.y(), to.z()), allHitResults);

    Vector<RayCastManifold> manifolds;
    if(allHitResults.hasHit())
    {
        for(int32_t i = 0; i < allHitResults.m_hitFractions.size(); ++i)
        {
            const btVector3& n = allHitResults.m_hitNormalWorld.at(i);
            const RigidbodyBullet& rigidBody = getRigidBodyFromCollisionObject(allHitResults.m_collisionObjects.at(i));
            manifolds.emplace_back(allHitResults.m_hitFractions.at(i), V3(n.x(), n.y(), n.z()), sp<Rigidbody>::make(rigidBody.makeShadow()));
        }
    }
    return manifolds;
}

btDiscreteDynamicsWorld* ColliderBullet::btDynamicWorld() const
{
    return _stub->_dynamics_world.get();
}

const HashMap<TypeId, sp<CollisionShapeRef>>& ColliderBullet::collisionShapes() const
{
    return _stub->_collision_shapes;
}

HashMap<TypeId, sp<CollisionShapeRef>>& ColliderBullet::collisionShapes()
{
    return _stub->_collision_shapes;
}

void ColliderBullet::markForDestroy(sp<CollisionObjectRef> collisionBody) const
{
    _stub->_mark_for_destroys.emplace_back(std::move(collisionBody));
}

void ColliderBullet::myInternalPreTickCallback(btDynamicsWorld* dynamicsWorld, btScalar /*timeStep*/)
{
    const ColliderBullet* self = static_cast<ColliderBullet*>(dynamicsWorld->getWorldUserInfo());
    const uint64_t timestamp = self->_stub->_timestamp;
    for(const GhostObject& i : self->_stub->_ghost_objects)
    {
        i._position.update(timestamp);
        i._quaternion.update(timestamp);
        const V3 pos = i._position.val();
        const V4 quaternion = i._quaternion.val();
        btTransform transform;
        transform.setIdentity();
        transform.setRotation(btQuaternion(quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w()));
        transform.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));
        i._bt_rigidbody_ref.collisionObjectRef()->collisionObject()->setWorldTransform(transform);
    }

    for(auto iter = self->_stub->_mark_for_destroys.begin(); iter != self->_stub->_mark_for_destroys.end(); )
        if(const auto& i = *iter; i->destroyCountDown(dynamicsWorld))
            iter = self->_stub->_mark_for_destroys.erase(iter);
        else
            ++ iter;
}

void ColliderBullet::myInternalTickCallback(btDynamicsWorld* dynamicsWorld, btScalar /*timeStep*/)
{
    btDispatcher* dispatcher = dynamicsWorld->getDispatcher();
    const int32_t numManifolds = dispatcher->getNumManifolds();
    btPersistentManifold** manifolds = dispatcher->getInternalManifoldPointer();
    ColliderBullet* self = static_cast<ColliderBullet*>(dynamicsWorld->getWorldUserInfo());
    for(int32_t i = 0; i < numManifolds; ++i)
    {
        if(const btPersistentManifold* contactManifold = manifolds[i]; contactManifold->getNumContacts() > 0)
        {
            const RigidbodyBullet& objA = getRigidBodyFromCollisionObject(contactManifold->getBody0());
            const RigidbodyBullet& objB = getRigidBodyFromCollisionObject(contactManifold->getBody1());
            if(objA.validate() && objB.validate())
            {
                const sp<CollisionCallback>& ccObjA = objA.collisionCallback();
                const sp<CollisionCallback>& ccObjB = objB.collisionCallback();

                if(ccObjA || ccObjB)
                {
                    const btManifoldPoint& pt = contactManifold->getContactPoint(0);
                    const btVector3& ptA = pt.getPositionWorldOnA();
                    const btVector3& normalOnB = pt.m_normalWorldOnB;
                    const V3 cp(ptA.x(), ptA.y(), ptA.z());
                    const V3 normal(normalOnB.x(), normalOnB.y(), normalOnB.z());

                    const sp<CollisionObjectRef>& refA = objA.collisionObjectRef();
                    const sp<CollisionObjectRef>& refB = objB.collisionObjectRef();

                    if(ccObjA && objB.stub()->_type != Rigidbody::BODY_TYPE_SENSOR)
                        self->addTickContactInfo(refA, ccObjA, refB, cp, normal);
                    if(ccObjB && objA.stub()->_type != Rigidbody::BODY_TYPE_SENSOR)
                        self->addTickContactInfo(refB, ccObjB, refA, cp, -normal);
                }
            }
        }
    }

    for(auto iter = self->_contact_infos.begin(); iter != self->_contact_infos.end(); )
    {
        const sp<CollisionObjectRef>& rigidBody = iter->first;
        ContactInfo& contactInfo = iter->second;
        if(const RigidbodyBullet& obj = getRigidBodyFromCollisionObject(rigidBody->collisionObject()); obj.collisionCallback())
            for(const sp<CollisionObjectRef>& i : contactInfo._last_tick)
                if(i->collisionObject())
                {
                    if(!contactInfo._current_tick.contains(i))
                        obj.collisionCallback()->onEndContact(getRigidBodyFromCollisionObject(i->collisionObject()).makeShadow());
                }

        contactInfo._last_tick = std::move(contactInfo._current_tick);
        if(contactInfo._last_tick.empty())
            iter = self->_contact_infos.erase(iter);
        else
            ++iter;
    }
}

void ColliderBullet::addTickContactInfo(const sp<CollisionObjectRef>& rigidBody, const sp<CollisionCallback>& callback, const sp<CollisionObjectRef>& contact, const V3& cp, const V3& normal)
{
    ContactInfo& contactInfo = _contact_infos[rigidBody];
    contactInfo._current_tick.insert(contact);
    if(!contactInfo._last_tick.contains(contact))
        if(const RigidbodyBullet& obj = getRigidBodyFromCollisionObject(contact->collisionObject()); obj.validate())
            callback->onBeginContact(obj.makeShadow(), CollisionManifold(cp, normal));
}

ColliderBullet::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _gravity(Documents::getAttribute<V3>(manifest, "gravity", {0, -9.8f, 0})), _model_loader(factory.getBuilder<ModelLoader>(manifest, constants::MODEL_LOADER)),
      _resource_loader_context(resourceLoaderContext)
{
    for(const auto& i : manifest->children("import"))
        _importers.emplace_back(factory.ensureBuilder<RigidbodyImporter>(i), i);
}

sp<ColliderBullet> ColliderBullet::BUILDER_IMPL1::build(const Scope& args)
{
    const sp<ColliderBullet> collider = sp<ColliderBullet>::make(_gravity, _model_loader.build(args));
    for(const auto& [k, v] : _importers)
        k->build(args)->import(collider, v);
    _resource_loader_context->renderController()->addPreComposeUpdatable(collider->_stub, BooleanType::__or__(_resource_loader_context->discarded(), sp<Boolean>::make<BooleanByWeakRef<ColliderBullet>>(collider, 0)));
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

}
