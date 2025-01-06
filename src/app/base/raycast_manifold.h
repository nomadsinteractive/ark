#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API RayCastManifold {
public:
    RayCastManifold() = default;
    RayCastManifold(float distance, const V3& normal, sp<Rigidbody> rigidBody = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RayCastManifold);

//  [[script::bindings::property]]
    float distance() const;
//  [[script::bindings::property]]
    const V3& normal() const;
//  [[script::bindings::property]]
    const sp<Rigidbody>& rigidbody() const;
    void setRigidBody(sp<Rigidbody> rigidbody);

private:
    float _distance;
    V3 _normal;
    sp<Rigidbody> _rigidbody;
};

}

