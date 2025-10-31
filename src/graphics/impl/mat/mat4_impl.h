#pragma once

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Mat4Impl final : public Mat4 {
public:
    Mat4Impl(const M4& mat = M4::identity()) noexcept;
    Mat4Impl(sp<Mat4> other) noexcept;
    Mat4Impl(V4 t, V4 b, V4 n, V4 w) noexcept;
    Mat4Impl(sp<Vec4> t, sp<Vec4> b, sp<Vec4> n, sp<Vec4> w) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat4Impl);

    M4 val() override;
    bool update(uint32_t tick) override;

    void set(const M4& mat);
    void set(sp<Mat4> mat);

private:
    sp<VariableWrapper<M4>> _impl;
};

}
