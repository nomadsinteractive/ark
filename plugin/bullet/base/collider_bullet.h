#ifndef ARK_PLUGIN_BULLET_BASE_COLLIDER_BULLET_H_
#define ARK_PLUGIN_BULLET_BASE_COLLIDER_BULLET_H_

#include <vector>

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/types/owned_ptr.h"
#include "core/types/safe_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "plugin/bullet/forwarding.h"
#include "plugin/bullet/api.h"

#include "btBulletDynamicsCommon.h"

namespace ark {
namespace plugin {
namespace bullet {

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
    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size = nullptr, const sp<Rotation>& rotate = nullptr) override;

//  [[script::bindings::auto]]
    void rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback);
//  [[script::bindings::auto]]
    void rayCastAllHit(const V3& from, const V3& to, const sp<CollisionCallback>& callback);

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

        uint32_t _body_id_base;

        float _time_step;
    };

    static void myInternalTickCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);
    static RigidBodyBullet getRigidBodyFromCollisionObject(const btCollisionObject* collisionObject);

private:
    sp<Stub> _stub;
};

}
}
}

#endif
