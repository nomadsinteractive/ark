#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::extends(Vec4)]]
class ARK_API RotationAxisTheta final : public Vec4, public Wrapper<Vec4> {
public:
//  [[script::bindings::auto]]
    RotationAxisTheta(sp<Vec3> axis, sp<Numeric> theta);

    V4 val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::property]]
    const sp<Vec3>& axis() const;
//  [[script::bindings::property]]
    const sp<Numeric>& theta() const;

private:
    void doUpdate();

private:
    sp<Vec3> _axis;
    sp<Numeric> _theta;
    V4 _val;
};

}
