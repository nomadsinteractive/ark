#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API CollisionManifold {
public:
    CollisionManifold() = default;
    CollisionManifold(const V3& contactPoint, const V3& normal);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(CollisionManifold);

//  [[script::bindings::property]]
    const V3& contactPoint() const;
//  [[script::bindings::property]]
    const V3& normal() const;

private:
    V3 _contact_point;
    V3 _normal;
};

}
