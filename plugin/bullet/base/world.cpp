#include "bullet/base/world.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/base/v3.h"

#include "bullet/rigid_body/rigid_body_bullet.h"

namespace ark {
namespace plugin {
namespace bullet {

World::World(const V3& gravity)
    : _stub(sp<Stub>::make(gravity))
{
}

sp<RigidBody> World::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotate>& rotate)
{
//    return sp<RigidBodyBullet>::make(*this, );
    return nullptr;
}

void World::run()
{
    _stub->step();
}

btDiscreteDynamicsWorld* World::btDynamicWorld() const
{
    return _stub->_dynamics_world.get();
}

World::Stub::Stub(const V3& gravity)
    : _collision_configuration(new btDefaultCollisionConfiguration()), _collision_dispatcher(new btCollisionDispatcher(_collision_configuration)), _broadphase(new btDbvtBroadphase()),
      _solver(new btSequentialImpulseConstraintSolver()), _dynamics_world(new btDiscreteDynamicsWorld(_collision_dispatcher, _broadphase, _solver, _collision_configuration)),
      _time_step(1 / 60.0f)
{
    _dynamics_world->setGravity(btVector3(gravity.x(), gravity.y(), gravity.z()));
}

World::Stub::~Stub()
{
    dispose();
}

void World::Stub::step()
{
    _dynamics_world->stepSimulation(_time_step);
}

void World::Stub::dispose()
{
    for(int32_t i = _dynamics_world->getNumConstraints() - 1; i >= 0; i--)
    {
        _dynamics_world->removeConstraint(_dynamics_world->getConstraint(i));
    }
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

World::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest)
    : _gravity(factory.ensureBuilder<Vec3>(manifest, "gravity"))
{
}

sp<World> World::BUILDER_IMPL1::build(const Scope& args)
{
    return sp<World>::make(_gravity->build(args)->val());
}

World::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& factory, const document& manifest)
    : _delegate(factory, manifest)
{
}

sp<Collider> World::BUILDER_IMPL2::build(const Scope& args)
{
    return _delegate.build(args);
}

}
}
}
