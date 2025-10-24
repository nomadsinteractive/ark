#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::extends(Vec4)]]
class ARK_API Rotation final : public Vec4 {
public:
//  [[script::bindings::auto]]
    Rotation(V4 quaternion);
//  [[script::bindings::auto]]
    Rotation(sp<Vec4> quaternion);
    Rotation(sp<Vec4> delegate, sp<Wrapper<Vec4>> wrapper);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotation);

    V4 val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::auto]]
    void reset(sp<Vec4> quaternion);

//  [[script::bindings::auto]]
    void setAxisTheta(V3 axis, float theta);
//  [[script::bindings::auto]]
    void setAxisTheta(sp<Vec3> axis, sp<Numeric> theta);

//  [[script::bindings::auto]]
    void setEuler(float pitch, float yaw, float roll);
//  [[script::bindings::auto]]
    void setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll);

//  [[script::bindings::auto]]
    sp<Vec3> applyTo(sp<Vec3> v) const;
//  [[script::bindings::auto]]
    sp<Vec4> applyTo(sp<Vec4> v) const;

//  [[script::bindings::auto]]
    sp<Mat4> toMatrix() const;
//  [[script::bindings::auto]]
    sp<Rotation> freeze() const;

//  [[script::bindings::operator(*)]]
    static sp<Rotation> mul(sp<Rotation> lhs, sp<Rotation> rhs);

//  [[script::bindings::auto]]
    static sp<Rotation> axisTheta(sp<Vec3> axis, sp<Numeric> theta);
//  [[script::bindings::auto]]
    static sp<Rotation> eulerAngle(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll);
//  [[script::bindings::auto]]
    static sp<Rotation> vectorReorientation(sp<Vec3> u, sp<Vec3> v);

private:
    Rotation(const sp<Vec4Wrapper>& vec4Wrapper);

private:
    sp<Vec4> _delegate;
    sp<Wrapper<Vec4>> _wrapper;
};

}
