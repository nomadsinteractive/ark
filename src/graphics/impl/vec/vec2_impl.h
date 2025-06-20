#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v2.h"
#include "graphics/forwarding.h"

namespace ark {

class ARK_API Vec2Impl final : public Vec2 {
public:
    Vec2Impl() noexcept;
    Vec2Impl(float x, float y) noexcept;
    Vec2Impl(sp<Numeric> v) noexcept;
    Vec2Impl(sp<Numeric> x, sp<Numeric> y) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec2Impl);

    V2 val() override;
    bool update(uint64_t timestamp) override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;

    void set(V2 val) const;

//  [[plugin::builder("vec2")]]
    class BUILDER final : public Builder<Vec2> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Vec2> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x;
        SafeBuilder<Numeric> _y;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Vec2> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        sp<Vec2> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x;
        sp<Builder<Numeric>> _y;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
};

}
