#include "app/inf/rigidbody_controller.h"

#include "core/ark.h"
#include "core/base/future.h"
#include "core/types/shared_ptr.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

class UpdatableApplyLinearVelocity final : public Updatable {
public:
    UpdatableApplyLinearVelocity(sp<RigidbodyController> rigidbodyController, sp<Vec3> linearVelocity)
        : _rigidbody_controller(std::move(rigidbodyController)), _linear_velocity(std::move(linearVelocity)) {
    }

    bool update(const uint32_t tick) override
    {
        if(_linear_velocity->update(tick))
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

    bool update(const uint32_t tick) override
    {
        if(_central_force->update(tick))
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

sp<Future> RigidbodyController::applyLinearVelocity(sp<RigidbodyController> self, sp<Vec3> linearVelocity, sp<Future> future)
{
    if(!future)
        future = sp<Future>::make();
    Ark::instance().renderController()->addPreComposeUpdatable(sp<Updatable>::make<UpdatableApplyLinearVelocity>(std::move(self), std::move(linearVelocity)), future->isDoneOrCanceled());
    return future;
}

sp<Future> RigidbodyController::applyCentralForce(const sp<RigidbodyController>& self, sp<Vec3> force, sp<Future> future)
{
    if(!future)
        future = sp<Future>::make();
    Ark::instance().renderController()->addPreComposeUpdatable(sp<Updatable>::make<UpdatableApplyCentralForce>(std::move(self), std::move(force)), future->isDoneOrCanceled());
    return future;
}

}
