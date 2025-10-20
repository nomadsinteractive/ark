#pragma once

#include "graphics/forwarding.h"

namespace ark {

class RigidbodyController {
public:
    virtual ~RigidbodyController() = default;

    virtual bool active() const = 0;
    virtual void setActive(bool active) = 0;

    virtual V3 linearVelocity() const = 0;
    virtual void setLinearVelocity(V3 velocity) = 0;

    virtual V3 linearFactor() const = 0;
    virtual void setLinearFactor(V3 factor) = 0;

    virtual V3 angularVelocity() const = 0;
    virtual void setAngularVelocity(V3 velocity) = 0;

    virtual V3 angularFactor() const = 0;
    virtual void setAngularFactor(V3 factor) = 0;

    virtual float friction() const = 0;
    virtual void setFriction(float friction) = 0;

    virtual float mass() const = 0;
    virtual void setMass(float mass) = 0;

    virtual void applyCentralForce(V3 force) = 0;
    virtual void applyCentralImpulse(V3 impulse) = 0;
};

}