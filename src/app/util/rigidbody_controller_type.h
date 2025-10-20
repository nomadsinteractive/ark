#pragma once

#include "core/forwarding.h"

#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("RigidbodyController")]]
class ARK_API RigidbodyControllerType final {
public:
//  [[script::bindings::property]]
    static bool active(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setActive(const sp<RigidbodyController>& self, bool active);

//  [[script::bindings::property]]
    static V3 linearVelocity(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setLinearVelocity(const sp<RigidbodyController>& self, V3 velocity);

//  [[script::bindings::property]]
    static V3 linearFactor(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setLinearFactor(const sp<RigidbodyController>& self, V3 factor);

//  [[script::bindings::property]]
    static V3 angularVelocity(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setAngularVelocity(const sp<RigidbodyController>& self, V3 velocity);

//  [[script::bindings::property]]
    static V3 angularFactor(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setAngularFactor(const sp<RigidbodyController>& self, V3 factor);

//  [[script::bindings::property]]
    static float friction(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setFriction(const sp<RigidbodyController>& self, float friction);

//  [[script::bindings::property]]
    static float mass(const sp<RigidbodyController>& self);
//  [[script::bindings::property]]
    static void setMass(const sp<RigidbodyController>& self, float mass);

//  [[script::bindings::classmethod]]
    static sp<Future> applyLinearVelocity(sp<RigidbodyController> self, sp<Vec3> linearVelocity);

//  [[script::bindings::classmethod]]
    static sp<Future> applyCentralForce(const sp<RigidbodyController>& self, V3 force);
//  [[script::bindings::classmethod]]
    static sp<Future> applyCentralForce(const sp<RigidbodyController>& self, sp<Vec3> force);
//  [[script::bindings::classmethod]]
    static void applyCentralImpulse(const sp<RigidbodyController>& self, V3 impulse);
};
    
}
