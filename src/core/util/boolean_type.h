#ifndef ARK_CORE_UTIL_BOOLEAN_TYPE_H_
#define ARK_CORE_UTIL_BOOLEAN_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Boolean")]]
class ARK_API BooleanType {
public:
//[[script::bindings::constructor]]
    static sp<Boolean> create(const sp<Boolean>& value);
//[[script::bindings::constructor]]
    static sp<Boolean> create(bool value);

//[[script::bindings::operator(&&)]]
    static sp<Boolean> __and__(sp<Boolean> self, sp<Boolean> rvalue);
//[[script::bindings::operator(||)]]
    static sp<Boolean> __or__(sp<Boolean> self, sp<Boolean> rvalue);
//[[script::bindings::operator(bool)]]
    static bool toBool(const sp<Boolean>& self);

//[[script::bindings::property]]
    static bool val(const sp<Boolean>& self);
//[[script::bindings::property]]
    static const sp<Boolean>& wrapped(const sp<Boolean>& self);

//[[script::bindings::classmethod]]
    static sp<Boolean> negative(sp<Boolean> self);

//[[script::bindings::classmethod]]
    static void set(const sp<Boolean::Impl>& self, bool value);
//[[script::bindings::classmethod]]
    static void set(const sp<BooleanWrapper>& self, bool value);
//[[script::bindings::classmethod]]
    static void set(const sp<BooleanWrapper>& self, const sp<Boolean>& delegate);

//[[script::bindings::classmethod]]
    static void toggle(const sp<Boolean::Impl>& self);
//[[script::bindings::classmethod]]
    static void toggle(const sp<BooleanWrapper>& self);

//[[script::bindings::classmethod]]
    static sp<Boolean> observe(const sp<Boolean>& self, const sp<Observer>& observer);

//[[script::bindings::classmethod]]
    static sp<Boolean> dye(sp<Boolean> self, sp<Boolean> condition = nullptr, String message = "");

//[[script::bindings::classmethod]]
    static void fix(const sp<Boolean>& self);

    static const sp<Boolean> TRUE;
    static const sp<Boolean> FALSE;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Boolean> {
    public:
        DICTIONARY(BeanFactory&, const String& expr);

        virtual sp<Boolean> build(const Scope& args) override;

    private:
        sp<Builder<Boolean>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Boolean> build(const Scope& args) override;

    private:
        String getValue(const document& manifest) const;

    private:
        sp<Builder<Boolean>> _value;
    };
};

}

#endif
