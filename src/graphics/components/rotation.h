#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::extends(Vec4)]]
class ARK_API Rotation final : public Vec4, public Wrapper<Vec4> {
public:
//  [[script::bindings::auto]]
    Rotation(const V4& quaternion);
//  [[script::bindings::auto]]
    Rotation(sp<Vec4> quaternion);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotation);

    V4 val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::auto]]
    void reset(sp<Vec4> quaternion);

//  [[script::bindings::auto]]
    void setRotation(const V3& axis, float theta);
//  [[script::bindings::auto]]
    void setRotation(sp<Vec3> axis, sp<Numeric> theta);
//  [[script::bindings::auto]]
    sp<RotationAxisTheta> getAxisTheta() const;

//  [[script::bindings::auto]]
    void setEuler(float pitch, float yaw, float roll);
//  [[script::bindings::auto]]
    void setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll);
//  [[script::bindings::auto]]
    sp<RotationEuler> getEuler() const;

//  [[script::bindings::classmethod]]
    static sp<Mat4> toMatrix(sp<Vec4> self);
};

}
