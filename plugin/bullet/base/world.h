#ifndef ARK_PLUGIN_BULLET_BASE_WORLD_H_
#define ARK_PLUGIN_BULLET_BASE_WORLD_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "btBulletDynamicsCommon.h"

namespace ark {
namespace plugin {
namespace bullet {

class World : public Runnable, public Collider, Implements<World, Runnable, Collider> {
public:
    World(const V3& gravity);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotate>& rotate) override;

    virtual void run() override;

    btDiscreteDynamicsWorld* btDynamicWorld() const;

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<World> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest);

        virtual sp<World> build(const Scope& args) override;

    private:
        sp<Builder<Vec3>> _gravity;
    };

//  [[plugin::builder("bullet")]]
    class BUILDER_IMPL2 : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

private:
    struct Stub {
        Stub(const V3& gravity);
        ~Stub();

        void step();
        void dispose();

        op<btDefaultCollisionConfiguration> _collision_configuration;
        op<btCollisionDispatcher> _collision_dispatcher;

        op<btBroadphaseInterface> _broadphase;
        op<btConstraintSolver> _solver;
        op<btDiscreteDynamicsWorld> _dynamics_world;

        float _time_step;
    };

private:
    sp<Stub> _stub;
};

}
}
}

#endif
