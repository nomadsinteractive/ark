#pragma once

#include "graphics/forwarding.h"

namespace ark {

class RigidbodyController {
public:
    virtual ~RigidbodyController() = default;

//  [[script::bindings::property]]
    virtual V3 linearVelocity() const = 0;
//  [[script::bindings::property]]
    virtual void setLinearVelocity(const V3& velocity) = 0;

//  [[script::bindings::auto]]
    virtual void applyCentralImpulse(const V3& impulse) = 0;

};

}