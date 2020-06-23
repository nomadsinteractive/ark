#include "bullet/rigid_body/rigid_body_bullet.h"

#include "bullet/base/collision_shape.h"

namespace ark {
namespace plugin {
namespace bullet {

RigidBodyBullet::Stub::Stub(ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass)
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

    btRigidBody* rigidBody = new btRigidBody(cInfo);
    _world.btDynamicWorld()->addRigidBody(rigidBody);
    rigidBody->setUserIndex(-1);
    return rigidBody;
}

RigidBodyBullet::RigidBodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass)
    : RigidBody(id, type, nullptr, nullptr, nullptr),  _stub(sp<Stub>::make(std::move(world), std::move(shape), transform, mass))
{
    stub()->_position = sp<Position>::make(_stub);
    stub()->_transform = sp<Transform>::make(sp<TransformDelegate>::make(_stub));
}

void RigidBodyBullet::dispose()
{
}

void RigidBodyBullet::applyCentralForce(const V3& force)
{
    _stub->_rigid_body->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

RigidBodyBullet::Position::Position(const sp<RigidBodyBullet::Stub>& stub)
    : _stub(stub)
{
}

bool RigidBodyBullet::Position::update(uint64_t /*timestamp*/)
{
    return true;
}

V3 RigidBodyBullet::Position::val()
{
    return getWorldPosition();
}

V3 RigidBodyBullet::Position::getWorldPosition() const
{
    btTransform transform;
    _stub->_motion_state->getWorldTransform(transform);
    const btVector3& pos = transform.getOrigin();
    return V3(pos.x(), pos.y(), pos.z());
}

RigidBodyBullet::TransformDelegate::TransformDelegate(const sp<RigidBodyBullet::Stub>& stub)
    : _stub(stub)
{
}

void RigidBodyBullet::TransformDelegate::snapshot(const Transform& /*transform*/, Transform::Snapshot& snapshot) const
{
    btTransform transform;
    _stub->_motion_state->getWorldTransform(transform);
    transform.setOrigin(btVector3(0, 0, 0));
    M4 matrix;
    transform.getOpenGLMatrix(reinterpret_cast<btScalar*>(&matrix));
    *snapshot.makeData<M4>() = matrix;
}

V3 RigidBodyBullet::TransformDelegate::transform(const Transform::Snapshot& /*snapshot*/, const V3& position) const
{
    return position;
}

M4 RigidBodyBullet::TransformDelegate::toMatrix(const Transform::Snapshot& snapshot) const
{
    return *snapshot.getData<M4>();
}

}
}
}
