#ifndef ARK_GRAPHICS_IMPL_MAT_MAT2_H_
#define ARK_GRAPHICS_IMPL_MAT_MAT2_H_

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Mat2Impl final : public Mat2 {
public:
    Mat2Impl() noexcept;
    Mat2Impl(const V2& t, const V2& b) noexcept;
    Mat2Impl(const sp<Vec2>& t, const sp<Vec2>& b) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat2Impl);

    virtual M2 val() override;
    virtual bool update(uint64_t timestamp) override;

    void fix();

private:
    sp<VariableWrapper<M2>> _impl;
};

}

#endif
