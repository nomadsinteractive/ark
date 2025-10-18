#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API RotationAxisTheta final : public Vec4, public Wrapper<Vec4> {
public:
    RotationAxisTheta(sp<Vec3> axis, sp<Numeric> theta);

    V4 val() override;
    bool update(uint64_t timestamp) override;

    const sp<Vec3>& axis() const;
    const sp<Numeric>& theta() const;

//  [[script::bindings::constructor]]
    static sp<Rotation> create(sp<Vec3> axis, sp<Numeric> theta);

private:
    void doUpdate();

private:
    sp<Vec3> _axis;
    sp<Numeric> _theta;
    V4 _val;
};

}
