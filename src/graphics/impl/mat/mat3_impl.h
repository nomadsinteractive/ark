#pragma once

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

namespace ark {

class ARK_API Mat3Impl final : public Mat3 {
public:
    Mat3Impl(const M3& mat = M3::identity()) noexcept;
    Mat3Impl(const V3& t, const V3& b, const V3& n) noexcept;
    Mat3Impl(const sp<Vec3>& t, const sp<Vec3>& b, const sp<Vec3>& n) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat3Impl);

    M3 val() override;
    bool update(uint64_t timestamp) override;

private:
    sp<VariableWrapper<M3>> _impl;
};

}
