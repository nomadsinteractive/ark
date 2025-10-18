#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API RotationEuler final : public Vec4, public Wrapper<Vec4> {
public:
    RotationEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll);

    V4 val() override;
    bool update(uint64_t timestamp) override;

    const sp<Numeric>& pitch() const;
    const sp<Numeric>& yaw() const;
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
