#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/builder/safe_builder.h"
#include "core/inf/variable.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Vec2i")]]
class ARK_API Vec2iType {
public:
//  [[script::bindings::constructor]]
    static sp<Vec2i> create(int32_t x, int32_t y);
//  [[script::bindings::constructor]]
    static sp<Vec2i> create(sp<Integer> x, sp<Integer> y = nullptr);

//  [[script::bindings::property]]
    static V2i val(const sp<Vec2i>& self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<Vec2i>& self);
//  [[script::bindings::seq(get)]]
    static Optional<int32_t> getItem(const sp<Vec2i>& self, ptrdiff_t index);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Vec2i> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Vec2i> build(const Scope& args) override;

    private:
        SafeBuilder<Integer> _x, _y;
        sp<Builder<Vec2i>> _value;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Vec2i> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        sp<Vec2i> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> _x, _y;
    };
};

}
