#include "bullet/rigid_body/rigid_body_bullet.h"

#include "bullet/base/collision_shape.h"

namespace ark {
namespace plugin {
namespace bullet {

RigidBodyBullet::Stub::Stub(World world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass)
    : _world(std::move(world)), _shape(std::move(shape)), _motion_state(makeMotionState(transform)), _rigid_body(makeRigidBody(_shape->btShape(), _motion_state.get(), mass))
{
}

RigidBodyBullet::Stub::~Stub()
{
    _world.btDynamicWorld()->removeRigidBody(_rigid_body);
}

btMotionState* RigidBodyBullet::Stub::makeMotionState(const btTransform& transform) const
{
    return new btDefaultMotionState(transform);
}

btRigidBody* RigidBodyBullet::Stub::makeRigidBody(btCollisionShape* shape, btMotionState* motionState, btScalar mass) const
{
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if(isDynamic)
        _shape->btShape()->calculateLocalInertia(mass, localInertia);

    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, localInertia);

    return new btRigidBody(cInfo);
}

RigidBodyBullet::RigidBodyBullet(int32_t id, Collider::BodyType type, World world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass)
    : RigidBody(id, type, nullptr, nullptr, nullptr),  _stub(sp<Stub>::make(std::move(world), std::move(shape), transform, mass))
{
}

void RigidBodyBullet::dispose()
{
}

}
}
}
