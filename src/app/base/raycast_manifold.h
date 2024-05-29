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
    RayCastManifold(float distance, const V3& normal, sp<RigidBodyRef> rigidBody);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RayCastManifold);

//  [[script::bindings::property]]
    float distance() const;
//  [[script::bindings::property]]
    const V3& normal() const;
//  [[script::bindings::property]]
    const sp<RigidBodyRef>& rigidBody() const;

private:
    float _distance;
    V3 _normal;
    sp<RigidBodyRef> _rigid_body;
};

}

