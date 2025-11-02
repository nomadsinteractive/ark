#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class RigidbodyController {
public:
    virtual ~RigidbodyController() = default;

//  [[script::bindings::property]]
    virtual bool active() const = 0;
//  [[script::bindings::property]]
    virtual void setActive(bool active) = 0;

//  [[script::bindings::property]]
    virtual V3 centralForce() const = 0;
//  [[script::bindings::property]]
    virtual void setCentralForce(const V3& force) = 0;

//  [[script::bindings::property]]
    virtual V3 linearVelocity() const = 0;
//  [[script::bindings::property]]
    virtual void setLinearVelocity(const V3& velocity) = 0;

//  [[script::bindings::property]]
    virtual V3 linearFactor() const = 0;
//  [[script::bindings::property]]
    virtual void setLinearFactor(const V3& factor) = 0;

//  [[script::bindings::property]]
    virtual V3 angularVelocity() const = 0;
//  [[script::bindings::property]]
    virtual void setAngularVelocity(const V3& velocity) = 0;

//  [[script::bindings::property]]
    virtual V3 angularFactor() const = 0;
//  [[script::bindings::property]]
    virtual void setAngularFactor(const V3& factor) = 0;

//  [[script::bindings::property]]
    virtual float friction() const = 0;
//  [[script::bindings::property]]
    virtual void setFriction(float friction) = 0;

//  [[script::bindings::property]]
    virtual float mass() const = 0;
//  [[script::bindings::property]]
    virtual void setMass(float mass) = 0;

    virtual void applyCentralForce(const V3& force) = 0;
//  [[script::bindings::auto]]
    virtual void applyCentralImpulse(const V3& impulse) = 0;

//  [[script::bindings::classmethod]]
    static sp<Future> applyLinearVelocity(sp<RigidbodyController> self, sp<Vec3> linearVelocity, sp<Future> future = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Future> applyCentralForce(const sp<RigidbodyController>& self, sp<Vec3> force, sp<Future> future = nullptr);
};

}