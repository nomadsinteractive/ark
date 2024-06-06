#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("StringVar", "String")]]
class ARK_API StringType {
public:
//  [[script::bindings::constructor]]
    static sp<StringVar> create(sp<String> value);
//  [[script::bindings::constructor]]
    static sp<StringVar> create(sp<Integer> value);
//  [[script::bindings::constructor]]
    static sp<StringVar> create(sp<StringVar> value);
    static sp<StringVar> create();

//  [[script::bindings::property]]
    static String val(const sp<StringVar>& self);
//  [[script::bindings::property]]
    static sp<StringVar> wrapped(const sp<StringVar>& self);

//  [[script::bindings::classmethod]]
    static void set(const sp<StringVar::Impl>& self, sp<String> value);
//  [[script::bindings::classmethod]]
    static void set(const sp<StringVarWrapper>& self, sp<String> value);
//  [[script::bindings::classmethod]]
    static void set(const sp<StringVarWrapper>& self, sp<StringVar> delegate);

//  [[script::bindings::classmethod]]
    static sp<StringVar> ifElse(sp<StringVar> self, sp<Boolean> condition, sp<StringVar> negative);

//  [[script::bindings::classmethod]]
    static sp<StringVar> freeze(const sp<StringVar>& self);

//  [[script::bindings::classmethod]]
    static sp<StringVar> dye(sp<StringVar> self, sp<Boolean> condition = nullptr, String message = "");

//  [[script::bindings::auto]]
    static sp<StringVar> format(String format, const Scope& kwargs);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<StringVar> {
    public:
        DICTIONARY(BeanFactory&, const String& expr);

        virtual sp<StringVar> build(const Scope& args) override;

    private:
        sp<Builder<String>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<StringVar> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<StringVar> build(const Scope& args) override;

    private:
        String getValue(const document& manifest) const;

    private:
        sp<Builder<String>> _value;
    };
};

}
