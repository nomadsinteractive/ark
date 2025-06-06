#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/builder/safe_builder.h"
#include "core/inf/variable.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Vec3i")]]
class ARK_API Vec3iType {
public:
//  [[script::bindings::constructor]]
    static sp<Vec3i> create(int32_t x, int32_t y, int32_t z);
//  [[script::bindings::constructor]]
    static sp<Vec3i> create(sp<Integer> x, sp<Integer> y = nullptr, sp<Integer> z = nullptr);

//  [[script::bindings::property]]
    static V3i val(const sp<Vec3i>& self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<Vec3i>& self);
//  [[script::bindings::seq(get)]]
    static Optional<int32_t> getItem(const sp<Vec3i>& self, ptrdiff_t index);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Vec3i> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Vec3i> build(const Scope& args) override;

    private:
        SafeBuilder<Integer> _x, _y, _z;
        sp<Builder<Vec3i>> _value;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Vec3i> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        sp<Vec3i> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> _x, _y, _z;
    };
};

}
