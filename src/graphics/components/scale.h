#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::extends(Vec3)]]
class ARK_API Scale final : public Wrapper<Vec3>, public Vec3 {
public:
    //  [[script::bindings::constructor]]
    Scale(sp<Vec3> scale);

    bool update(uint64_t timestamp) override;
    V3 val() override;

//  [[script::bindings::auto]]
    void reset(sp<Vec3> scale);

//  [[script::bindings::auto]]
    sp<Mat4> toMatrix() const;
};

}
