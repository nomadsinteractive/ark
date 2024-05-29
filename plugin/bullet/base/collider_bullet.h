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
#include "core/types/weak_ptr.h"

#include "graphics/base/transform.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "plugin/bullet/forwarding.h"
#include "plugin/bullet/api.h"

#include "btBulletDynamicsCommon.h"

namespace ark::plugin::bullet {

//[[script::bindings::name("World")]]
class ARK_PLUGIN_BULLET_API ColliderBullet : public Runnable, public Collider, Implements<ColliderBullet, Runnable, Collider> {
public:
    class RigidBodyImporter {
    public:
        virtual ~RigidBodyImporter() = default;

        virtual void import(ColliderBullet& collider, const document& manifest) = 0;
    };

public:
    ColliderBullet(const V3& gravity, sp<ModelLoader> modelLoader);

//  [[script::bindings::auto]]
    virtual sp<RigidBody> createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotate = nullptr, sp<Boolean> discarded = nullptr) override;
//  [[script::bindings::auto]]
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) override;

//  [[script::bindings::auto]]
    void rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback, int32_t filterGroup = 1, int32_t filterMask = -1);

    virtual void run() override;

    btDiscreteDynamicsWorld* btDynamicWorld() const;

    const std::unordered_map<int32_t, sp<CollisionShape>>& collisionShapes() const;
    std::unordered_map<int32_t, sp<CollisionShape>>& collisionShapes();

//  [[plugin::resource-loader]]
    class BUILDER_IMPL1 : public Builder<ColliderBullet> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ColliderBullet> build(const Scope& args) override;

    private:
        sp<Builder<Vec3>> _gravity;
        SafePtr<Builder<ModelLoader>> _model_loader;
        std::vector<std::pair<sp<Builder<RigidBodyImporter>>, document>> _importers;

        sp<ResourceLoaderContext> _resource_loader_context;
    };

//  [[plugin::resource-loader("btWorld")]]
    class BUILDER_IMPL2 : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

private:
    struct KinematicObject {
        KinematicObject(sp<Vec3> position, sp<Rotation> rotation, sp<BtRigidBodyRef> rigidBody);

        SafePtr<Vec3> _position;
        SafePtr<Rotation> _rotation;
        sp<BtRigidBodyRef> _rigid_body;

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
        std::unordered_map<int32_t, sp<CollisionShape>> _collision_shapes;

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
        std::set<sp<BtRigidBodyRef>> _last_tick;
        std::set<sp<BtRigidBodyRef>> _current_tick;
    };

    class DynamicPosition : public Vec3 {
    public:
        DynamicPosition(const sp<btMotionState>& motionState, bool isStaticBody);

        virtual bool update(uint64_t timestamp) override;

        virtual V3 val() override;

    private:
        V3 getWorldPosition() const;

    private:
        sp<btMotionState> _motion_state;
        bool _is_static_body;
    };

    class DynamicTransform : public Transform::Delegate {
    public:
        DynamicTransform(const sp<btMotionState>& motionState);

        virtual void snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const override;
        virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
        virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

    private:
        sp<btMotionState> _motion_state;
    };

    static sp<BtRigidBodyRef> makeRigidBody(btDynamicsWorld* world, btCollisionShape* shape, btMotionState* motionState, Collider::BodyType bodyType, btScalar mass);
    static sp<BtRigidBodyRef> makeGhostObject(btDynamicsWorld* world, btCollisionShape* shape, Collider::BodyType bodyType);

    static void myInternalPreTickCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);
    static void myInternalTickCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);

    static RigidBodyBullet getRigidBodyFromCollisionObject(const btCollisionObject* collisionObject);

    void addTickContactInfo(const sp<BtRigidBodyRef>& rigidBody, const sp<CollisionCallback>& callback, const sp<BtRigidBodyRef>& contact, const V3& cp, const V3& normal);

private:
    sp<Stub> _stub;
    std::map<sp<BtRigidBodyRef>, ContactInfo> _contact_infos;
};

}
