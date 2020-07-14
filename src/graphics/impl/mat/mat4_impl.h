#ifndef ARK_GRAPHICS_IMPL_MAT_MAT4_H_
#define ARK_GRAPHICS_IMPL_MAT_MAT4_H_

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Mat4Impl final : public Mat4 {
public:
    Mat4Impl() noexcept;
    Mat4Impl(const V4& t, const V4& b, const V4& n, const V4& w) noexcept;
    Mat4Impl(const sp<Vec4>& t, const sp<Vec4>& b, const sp<Vec4>& n, const sp<Vec4>& w) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat4Impl);

    virtual M4 val() override;
    virtual bool update(uint64_t timestamp) override;

    void fix();

private:
    sp<VariableWrapper<M4>> _impl;
};

}

#endif
