#include "app/util/rigidbody_controller_type.h"

#include "core/ark.h"
#include "core/base/future.h"
#include "core/types/shared_ptr.h"

#include "app/base/application_context.h"
#include "app/inf/rigidbody_controller.h"

namespace ark {

namespace {

class UpdatableApplyLinearVelocity final : public Updatable {
public:
    UpdatableApplyLinearVelocity(sp<RigidbodyController> rigidbodyController, sp<Vec3> linearVelocity)
        : _rigidbody_controller(std::move(rigidbodyController)), _linear_velocity(std::move(linearVelocity)) {
    }

    bool update(const uint64_t timestamp) override
    {
        if(_linear_velocity->update(timestamp))
        {
            _rigidbody_controller->setLinearVelocity(_linear_velocity->val());
            return true;
        }
        return false;
    }

private:
    sp<RigidbodyController> _rigidbody_controller;
    sp<Vec3> _linear_velocity;
};

class UpdatableApplyCentralForce final : public Updatable {
public:
    UpdatableApplyCentralForce(sp<RigidbodyController> rigidbodyController, sp<Vec3> centralForce)
        : _rigidbody_controller(std::move(rigidbodyController)), _central_force(std::move(centralForce)) {
    }

    bool update(const uint64_t timestamp) override
    {
        if(_central_force->update(timestamp))
        {
            _rigidbody_controller->applyCentralForce(_central_force->val());
            return true;
        }
        return false;
    }

private:
    sp<RigidbodyController> _rigidbody_controller;
    sp<Vec3> _central_force;
};

}

bool RigidbodyControllerType::active(const sp<RigidbodyController>& self)
{
    return self->active();
}

void RigidbodyControllerType::setActive(const sp<RigidbodyController>& self, const bool active)
{
    self->setActive(active);
}

V3 RigidbodyControllerType::linearVelocity(const sp<RigidbodyController>& self)
{
    return self->linearVelocity();
}

void RigidbodyControllerType::setLinearVelocity(const sp<RigidbodyController>& self, const V3 velocity)
{
    self->setLinearVelocity(velocity);
}

V3 RigidbodyControllerType::centralForce(const sp<RigidbodyController>& self)
{
    return self->centralForce();
}

void RigidbodyControllerType::setCentralForce(const sp<RigidbodyController>& self, const V3 force)
{
    self->setCentralForce(force);
}

V3 RigidbodyControllerType::linearFactor(const sp<RigidbodyController>& self)
{
    return self->linearFactor();
}

void RigidbodyControllerType::setLinearFactor(const sp<RigidbodyController>& self, const V3 factor)
{
    self->setLinearFactor(factor);
}

V3 RigidbodyControllerType::angularVelocity(const sp<RigidbodyController>& self)
{
    return self->angularVelocity();
}

void RigidbodyControllerType::setAngularVelocity(const sp<RigidbodyController>& self, const V3 velocity)
{
    self->setAngularVelocity(velocity);
}

V3 RigidbodyControllerType::angularFactor(const sp<RigidbodyController>& self)
{
    return self->angularFactor();
}

void RigidbodyControllerType::setAngularFactor(const sp<RigidbodyController>& self, const V3 factor)
{
    self->setAngularFactor(factor);
}

float RigidbodyControllerType::friction(const sp<RigidbodyController>& self)
{
    return self->friction();
}

void RigidbodyControllerType::setFriction(const sp<RigidbodyController>& self, float friction)
{
    self->setFriction(friction);
}

float RigidbodyControllerType::mass(const sp<RigidbodyController>& self)
{
    return self->mass();
}

void RigidbodyControllerType::setMass(const sp<RigidbodyController>& self, const float mass)
{
    self->setMass(mass);
}

sp<Future> RigidbodyControllerType::applyLinearVelocity(sp<RigidbodyController> self, sp<Vec3> linearVelocity, sp<Future> future)
{
    if(!future)
        future = sp<Future>::make();
    Ark::instance().renderController()->addPreComposeUpdatable(sp<Updatable>::make<UpdatableApplyLinearVelocity>(std::move(self), std::move(linearVelocity)), future->isDoneOrCanceled());
    return future;
}

sp<Future> RigidbodyControllerType::applyCentralForce(const sp<RigidbodyController>& self, sp<Vec3> force, sp<Future> future)
{
    if(!future)
        future = sp<Future>::make();
    Ark::instance().renderController()->addPreComposeUpdatable(sp<Updatable>::make<UpdatableApplyCentralForce>(std::move(self), std::move(force)), future->isDoneOrCanceled());
    return future;
}

void RigidbodyControllerType::applyCentralImpulse(const sp<RigidbodyController>& self, const V3 impulse)
{
    self->applyCentralImpulse(impulse);
}

}
