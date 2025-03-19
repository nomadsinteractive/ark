#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Vec3Impl final : public Vec3 {
public:
    Vec3Impl() noexcept;
    Vec3Impl(sp<Numeric> v) noexcept;
    Vec3Impl(float x, float y, float z) noexcept;
    Vec3Impl(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec3Impl);

    V3 val() override;
    bool update(uint64_t timestamp) override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;
    const sp<NumericWrapper>& z() const;

    void set(const V3& val);
    void fix();

//  [[plugin::builder("vec3")]]
    class BUILDER final : public Builder<Vec3> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Vec3> build(const Scope& args) override;

    private:
        SafeBuilder<Numeric> _x, _y, _z;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Vec3> {
    public:
        DICTIONARY(BeanFactory& parent, const String& str);

        sp<Vec3> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z;
        String _str;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
    sp<NumericWrapper> _z;

};

}
