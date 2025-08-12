#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Vec4Impl final : public Vec4 {
public:
    Vec4Impl() noexcept;
    Vec4Impl(sp<Numeric> v) noexcept;
    Vec4Impl(float x, float y, float z, float w) noexcept;
    Vec4Impl(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z, sp<Numeric> w) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec4Impl);

    V4 val() override;
    bool update(uint64_t timestamp) override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;
    const sp<NumericWrapper>& z() const;
    const sp<NumericWrapper>& w() const;

    void set(V4 val) const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Vec4> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Vec4> build(const Scope& args) override;

    private:
        SafeBuilder<Numeric> _x, _y, _z, _w;
        sp<Builder<Vec4>> _value;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Vec4> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        sp<Vec4> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z, _w;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
    sp<NumericWrapper> _z;
    sp<NumericWrapper> _w;
};

}
