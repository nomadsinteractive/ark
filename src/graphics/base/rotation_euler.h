#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::extends(Vec4)]]
class ARK_API RotationEuler final : public Vec4, public Wrapper<Vec4> {
public:
//  [[script::bindings::auto]]
    RotationEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll);

    V4 val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::property]]
    const sp<Numeric>& pitch() const;
//  [[script::bindings::property]]
    const sp<Numeric>& yaw() const;
//  [[script::bindings::property]]
    const sp<Numeric>& roll() const;

private:
    void doUpdate();

private:
    sp<Numeric> _pitch;
    sp<Numeric> _yaw;
    sp<Numeric> _roll;
    V4 _val;
};

}
