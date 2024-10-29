#pragma once

#include <map>
#include <vector>

#include "core/forwarding.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/owned_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/base/transform.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "plugin/bullet/forwarding.h"
#include "plugin/bullet/api.h"

#include "btBulletDynamicsCommon.h"

namespace ark::plugin::bullet {

//[[script::bindings::name("World")]]
class ARK_PLUGIN_BULLET_API ColliderBullet final : public Runnable, public Collider, Implements<ColliderBullet, Runnable, Collider> {
public:
    class RigidbodyImporter {
    public:
        virtual ~RigidbodyImporter() = default;

        virtual void import(ColliderBullet& collider, const document& manifest) = 0;
    };

public:
    ColliderBullet(const V3& gravity, sp<ModelLoader> modelLoader);

//  [[script::bindings::auto]]
    sp<Rigidbody> createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotation = nullptr, sp<Boolean> discarded = nullptr) override;\
//  [[script::bindings::auto]]
    sp<Shape> createShape(const NamedType& type, sp<Vec3> size) override;
//  [[script::bindings::auto]]
    std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) override;

//  [[script::bindings::auto]]
    void rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback, int32_t filterGroup = 1, int32_t filterMask = -1) const;

    void run() override;

    btDiscreteDynamicsWorld* btDynamicWorld() const;

    const std::unordered_map<TypeId, sp<CollisionShape>>& collisionShapes() const;
    std::unordered_map<TypeId, sp<CollisionShape>>& collisionShapes();

//  [[plugin::resource-loader]]
    class BUILDER_IMPL1 : public Builder<ColliderBullet> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<ColliderBullet> build(const Scope& args) override;

    private:
        V3 _gravity;
        SafePtr<Builder<ModelLoader>> _model_loader;
        std::vector<std::pair<sp<Builder<RigidbodyImporter>>, document>> _importers;

        sp<ResourceLoaderContext> _resource_loader_context;
    };

//  [[plugin::resource-loader("btWorld")]]
    class BUILDER_IMPL2 : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Collider> build(const Scope& args) override;

    private:
        BUILDER_IMPL1 _impl;
    };

private:
    struct KinematicObject {
        KinematicObject(sp<Vec3> position, sp<Vec4> quaternion, sp<BtRigidbodyRef> rigidBody);

        SafeVar<Vec3> _position;
        SafeVar<Vec4> _quaternion;
        sp<BtRigidbodyRef> _rigid_body;

        class ListFilter {
        public:
            ListFilter() = default;

            FilterAction operator() (const KinematicObject& item) const;

        };
    };

    struct Stub {
        Stub(const V3& gravity, sp<ModelLoader> modelLoader);
        ~Stub();

        void step();
        void dispose();

        sp<ModelLoader> _model_loader;
        std::unordered_map<TypeId, sp<CollisionShape>> _collision_shapes;

        op<btDefaultCollisionConfiguration> _collision_configuration;
        op<btCollisionDispatcher> _collision_dispatcher;

        op<btBroadphaseInterface> _broadphase;
        op<btConstraintSolver> _solver;
        op<btDiscreteDynamicsWorld> _dynamics_world;

        List<KinematicObject, KinematicObject::ListFilter> _kinematic_objects;

        uint32_t _body_id_base;

        float _time_step;
    };

    struct ContactInfo {
        std::set<sp<BtRigidbodyRef>> _last_tick;
        std::set<sp<BtRigidbodyRef>> _current_tick;
    };

    static sp<BtRigidbodyRef> makeRigidBody(btDynamicsWorld* world, btCollisionShape* shape, btMotionState* motionState, Collider::BodyType bodyType, btScalar mass);
    static sp<BtRigidbodyRef> makeGhostObject(btDynamicsWorld* world, btCollisionShape* shape, Collider::BodyType bodyType);

    static void myInternalPreTickCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);
    static void myInternalTickCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);

    static const RigidbodyBullet& getRigidBodyFromCollisionObject(const btCollisionObject* collisionObject);

    void addTickContactInfo(const sp<BtRigidbodyRef>& rigidBody, const sp<CollisionCallback>& callback, const sp<BtRigidbodyRef>& contact, const V3& cp, const V3& normal);

private:
    sp<Stub> _stub;
    std::map<sp<BtRigidbodyRef>, ContactInfo> _contact_infos;
};

}
