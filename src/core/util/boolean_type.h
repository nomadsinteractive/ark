#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Boolean")]]
class ARK_API BooleanType {
public:
//  [[script::bindings::constructor]]
    static sp<Boolean> create(const sp<Boolean>& value);
//  [[script::bindings::constructor]]
    static sp<Boolean> create(bool value);

//  [[script::bindings::operator(&)]]
    static sp<Boolean> __and__(sp<Boolean> self, sp<Boolean> rvalue);
//  [[script::bindings::operator(|)]]
    static sp<Boolean> __or__(sp<Boolean> self, sp<Boolean> rvalue);
//  [[script::bindings::operator(bool)]]
    static bool toBool(const sp<Boolean>& self);

//  [[script::bindings::property]]
    static bool val(const sp<Boolean>& self);
//  [[script::bindings::property]]
    static sp<Boolean> wrapped(const sp<Boolean>& self);

//  [[script::bindings::classmethod]]
    static bool update(const sp<Boolean>& self);
//  [[script::bindings::classmethod]]
    static sp<Boolean> freeze(const sp<Boolean>& self);

//  [[script::bindings::classmethod]]
    static sp<Boolean> wrap(sp<Boolean> self);

//  [[script::bindings::classmethod]]
    static sp<Boolean> negative(sp<Boolean> self);

//  [[script::bindings::classmethod]]
    static sp<Boolean> expect(sp<Boolean> self, sp<Boolean> expectation, sp<Future> future);

//  [[script::bindings::classmethod]]
    static void set(const sp<BooleanWrapper>& self, sp<Boolean> value);
//  [[script::bindings::classmethod]]
    static void set(const sp<BooleanWrapper>& self, bool value);

//  [[script::bindings::classmethod]]
    static sp<Boolean> dye(sp<Boolean> self, sp<Boolean> condition = nullptr, String message = "");

//  [[script::bindings::type]]
    static sp<Boolean> InTick(sp<Integer> tickStart, sp<Integer> tickEnd = nullptr);

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Boolean> {
    public:
        DICTIONARY(BeanFactory&, const String& expr);

        sp<Boolean> build(const Scope& args) override;

    private:
        sp<Builder<Boolean>> _value;
    };
};

}
