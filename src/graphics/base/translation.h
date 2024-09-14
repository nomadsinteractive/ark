#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Translation final : public Vec3, public Wrapper<Vec3> {
public:
//  [[script::bindings::constructor]]
    Translation(sp<Vec3> translate);

    bool update(uint64_t timestamp) override;
    V3 val() override;

//  [[script::bindings::auto]]
    sp<Mat4> toMatrix() const;
};

}
