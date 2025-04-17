#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Quaternion final : public Vec4, public Wrapper<Vec4> {
public:
    Quaternion(sp<Vec4> quaternion);
    Quaternion(sp<Numeric> theta, sp<Vec3> axis);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Quaternion);

    V4 val() override;
    bool update(uint64_t timestamp) override;

    void setRotation(sp<Numeric> theta, sp<Vec3> axis);

    sp<Mat4> toMatrix() const;
};

}
