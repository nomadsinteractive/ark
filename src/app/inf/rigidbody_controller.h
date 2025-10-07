#pragma once

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
    virtual V3 linearVelocity() const = 0;
//  [[script::bindings::property]]
    virtual void setLinearVelocity(V3 velocity) = 0;

//  [[script::bindings::property]]
    virtual V3 angularVelocity() const = 0;
//  [[script::bindings::property]]
    virtual void setAngularVelocity(V3 velocity) = 0;

//  [[script::bindings::property]]
    virtual V3 angularFactor() const = 0;
//  [[script::bindings::property]]
    virtual void setAngularFactor(V3 factor) = 0;

//  [[script::bindings::property]]
    virtual float friction() const = 0;
//  [[script::bindings::property]]
    virtual void setFriction(float friction) = 0;

//  [[script::bindings::auto]]
    virtual void applyCentralImpulse(V3 impulse) = 0;

};

}