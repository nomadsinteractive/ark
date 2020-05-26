#include "bullet/base/collider_bullet.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/boolean_util.h"

#include "graphics/base/rotate.h"
#include "graphics/base/size.h"
#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"

#include "bullet/base/collision_shape.h"
#include "bullet/rigid_body/rigid_body_bullet.h"

namespace ark {
namespace plugin {
namespace bullet {

ColliderBullet::ColliderBullet(const V3& gravity)
    : _stub(sp<Stub>::make(gravity))
{
}

sp<RigidBody> ColliderBullet::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotate>& rotate)
{
    btTransform transform;
    const V3 pos = position->val();
    const Rotation rot = rotate ? rotate->val() : Rotation();
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));
    transform.setRotation(btQuaternion(btVector3(rot.direction.x(), rot.direction.y(), rot.direction.z()), rot.angle));
    sp<CollisionShape> cs = sp<CollisionShape>::make(*this, new btBoxShape(btVector3(size->width() / 2, size->height() / 2, size->depth() / 2)));
    return sp<RigidBodyBullet>::make(type, static_cast<Collider::BodyType>(shape), *this, std::move(cs), transform, type == Collider::BODY_TYPE_STATIC ? 0 : 1.0f);
}

void ColliderBullet::run()
{
    _stub->step();
}

btDiscreteDynamicsWorld* ColliderBullet::btDynamicWorld() const
{
    return _stub->_dynamics_world.get();
}

ColliderBullet::Stub::Stub(const V3& gravity)
    : _collision_configuration(new btDefaultCollisionConfiguration()), _collision_dispatcher(new btCollisionDispatcher(_collision_configuration)), _broadphase(new btDbvtBroadphase()),
      _solver(new btSequentialImpulseConstraintSolver()), _dynamics_world(new btDiscreteDynamicsWorld(_collision_dispatcher, _broadphase, _solver, _collision_configuration)),
      _time_step(1 / 60.0f)
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
    : _gravity(factory.ensureBuilder<Vec3>(manifest, "gravity")), _resource_loader_context(resourceLoaderContext)
{
}

sp<ColliderBullet> ColliderBullet::BUILDER_IMPL1::build(const Scope& args)
{
    const sp<ColliderBullet> collider = sp<ColliderBullet>::make(_gravity->build(args)->val());
    _resource_loader_context->renderController()->addPreUpdateRequest(collider, BooleanUtil::__or__(_resource_loader_context->disposed(), sp<Boolean>::make<BooleanByWeakRef<ColliderBullet>>(collider, 1)));
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
