#ifndef ARK_GRAPHICS_IMPL_MAT_MAT3_H_
#define ARK_GRAPHICS_IMPL_MAT_MAT3_H_

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Mat3Impl final : public Mat3 {
public:
    Mat3Impl() noexcept;
    Mat3Impl(const V3& t, const V3& b, const V3& n) noexcept;
    Mat3Impl(const sp<Vec3>& t, const sp<Vec3>& b, const sp<Vec3>& n) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat3Impl);

    virtual M3 val() override;

    void fix();

private:
    sp<VariableWrapper<M3>> _impl;

};

}

#endif
