#ifndef ARK_CORE_UTIL_BOOLEAN_UTIL_H_
#define ARK_CORE_UTIL_BOOLEAN_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Boolean")]]
class ARK_API BooleanUtil {
public:
//[[script::bindings::constructor]]
    static sp<Boolean> create(const sp<Boolean>& value);
//[[script::bindings::constructor]]
    static sp<Boolean> create(bool value);

//[[script::bindings::operator(&&)]]
    static sp<Boolean> __and__(const sp<Boolean>& self, const sp<Boolean>& rvalue);
//[[script::bindings::operator(||)]]
    static sp<Boolean> __or__(const sp<Boolean>& self, const sp<Boolean>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Boolean> negative(const sp<Boolean>& self);
//[[script::bindings::operator(bool)]]
    static bool toBool(const sp<Boolean>& self);

//[[script::bindings::property]]
    static bool val(const sp<Boolean>& self);
//[[script::bindings::property]]
    static void setVal(const sp<Boolean::Impl>& self, bool value);
//[[script::bindings::property]]
    static void setVal(const sp<BooleanWrapper>& self, bool value);
//[[script::bindings::property]]
    static const sp<Boolean>& delegate(const sp<Boolean>& self);
//[[script::bindings::property]]
    static void setDelegate(const sp<Boolean>& self, const sp<Boolean>& delegate);

//[[script::bindings::classmethod]]
    static void set(const sp<Boolean::Impl>& self, bool value);
//[[script::bindings::classmethod]]
    static void set(const sp<BooleanWrapper>& self, bool value);
//[[script::bindings::classmethod]]
    static void set(const sp<BooleanWrapper>& self, const sp<Boolean>& delegate);

//[[script::bindings::classmethod]]
    static void fix(const sp<Boolean>& self);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Boolean> {
    public:
        DICTIONARY(BeanFactory&, const String& value);

        virtual sp<Boolean> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Boolean> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _value;
    };
};

}

#endif