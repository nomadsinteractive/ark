#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v2.h"
#include "graphics/forwarding.h"

namespace ark {

class ARK_API Vec2Impl final : public Vec2 {
public:
    Vec2Impl() noexcept;
    Vec2Impl(float x, float y) noexcept;
    Vec2Impl(const V2& xy) noexcept;
    Vec2Impl(sp<Numeric> v) noexcept;
    Vec2Impl(sp<Numeric> x, sp<Numeric> y) noexcept;
    Vec2Impl(Vec2& other) noexcept;

    V2 val() override;
    bool update(uint64_t timestamp) override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;

    void set(const V2& val);
    void fix();

//  [[plugin::builder("vec2")]]
    class BUILDER : public Builder<Vec2> {
    public:
        BUILDER(BeanFactory& factory, const document& doc);

        virtual sp<Vec2> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x;
        SafePtr<Builder<Numeric>> _y;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Vec2> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        virtual sp<Vec2> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x;
        sp<Builder<Numeric>> _y;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;

};

}
