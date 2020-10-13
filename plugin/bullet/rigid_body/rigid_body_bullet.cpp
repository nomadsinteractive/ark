#include "bullet/rigid_body/rigid_body_bullet.h"

#include "bullet/base/collision_shape.h"
#include "bullet/base/bt_rigid_body_ref.h"

namespace ark {
namespace plugin {
namespace bullet {

RigidBodyBullet::Stub::Stub(ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, Collider::BodyType bodyType, btScalar mass)
    : _world(std::move(world)), _shape(std::move(shape)), _motion_state(makeMotionState(transform)), _rigid_body(makeRigidBody(_shape->btShape(), _motion_state.get(), bodyType, mass)), _body_type(bodyType)
{
}

RigidBodyBullet::Stub::~Stub()
{
    _world.btDynamicWorld()->removeRigidBody(_rigid_body->ptr());
    delete reinterpret_cast<WeakPtr<RigidBody::Stub>*>(_rigid_body->ptr()->getUserPointer());
    _rigid_body->ptr()->setUserPointer(nullptr);
    _rigid_body->reset();
}

btMotionState* RigidBodyBullet::Stub::makeMotionState(const btTransform& transform) const
{
    return new btDefaultMotionState(transform);
}

sp<BtRigidBodyRef> RigidBodyBullet::Stub::makeRigidBody(btCollisionShape* shape, btMotionState* motionState, Collider::BodyType bodyType, btScalar mass) const
{
    DASSERT(bodyType == Collider::BODY_TYPE_STATIC || bodyType == Collider::BODY_TYPE_DYNAMIC || bodyType == Collider::BODY_TYPE_KINEMATIC);
    btVector3 localInertia(0, 0, 0);
    if(mass != 0.f)
        _shape->btShape()->calculateLocalInertia(mass, localInertia);

    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, localInertia);

    btRigidBody* rigidBody = new btRigidBody(cInfo);
    if(bodyType == Collider::BODY_TYPE_KINEMATIC)
    {
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
    }
    _world.btDynamicWorld()->addRigidBody(rigidBody);
    rigidBody->setUserIndex(-1);
    return sp<BtRigidBodyRef>::make(rigidBody);
}

RigidBodyBullet::RigidBodyBullet(int32_t id, Collider::BodyType type, ColliderBullet world, sp<CollisionShape> shape, const btTransform& transform, btScalar mass)
    : RigidBody(id, type, nullptr, nullptr, nullptr, sp<Stub>::make(std::move(world), std::move(shape), transform, type, mass)), _stub(stub()->_impl.unpack<Stub>())
{
    stub()->_position = sp<Position>::make(_stub);
    stub()->_transform = sp<Transform>::make(sp<TransformDelegate>::make(_stub));
    _stub->_rigid_body->ptr()->setUserPointer(reinterpret_cast<void*>(new WeakPtr<RigidBody::Stub>(stub())));
}

RigidBodyBullet::RigidBodyBullet(sp<RigidBody::Stub> other)
    : RigidBody(std::move(other)), _stub(stub()->_impl.unpack<Stub>())
{
}

void RigidBodyBullet::dispose()
{
}

void RigidBodyBullet::applyCentralForce(const V3& force)
{
    _stub->_rigid_body->ptr()->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

V3 RigidBodyBullet::linearVelocity() const
{
    const btVector3& velocity = _stub->_rigid_body->ptr()->getLinearVelocity();
    return V3(velocity.x(), velocity.y(), velocity.z());
}

void RigidBodyBullet::setLinearVelocity(const V3& velocity)
{
    _stub->_rigid_body->ptr()->setActivationState(DISABLE_DEACTIVATION);
    _stub->_rigid_body->ptr()->setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));
}

float RigidBodyBullet::friction() const
{
    return _stub->_rigid_body->ptr()->getFriction();
}

void RigidBodyBullet::setFriction(float friction)
{
    _stub->_rigid_body->ptr()->setFriction(friction);
}

V3 RigidBodyBullet::angularFactor() const
{
    const btVector3& factor = _stub->_rigid_body->ptr()->getAngularFactor();
    return V3(factor.x(), factor.y(), factor.z());
}

void RigidBodyBullet::setAngularFactor(const V3& factor)
{
    _stub->_rigid_body->ptr()->setAngularFactor(btVector3(factor.x(), factor.y(), factor.z()));
}

const sp<BtRigidBodyRef>& RigidBodyBullet::rigidBody() const
{
    return _stub->_rigid_body;
}

RigidBodyBullet::Position::Position(const sp<RigidBodyBullet::Stub>& stub)
    : _stub(stub), _is_static_body(_stub->_body_type == Collider::BODY_TYPE_STATIC)
{
}

bool RigidBodyBullet::Position::update(uint64_t /*timestamp*/)
{
    return _is_static_body ? false : true;
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
