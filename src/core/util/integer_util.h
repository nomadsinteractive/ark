#ifndef ARK_CORE_UTIL_INTEGER_UTIL_H_
#define ARK_CORE_UTIL_INTEGER_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Integer")]]
class ARK_API IntegerUtil final {
public:

//[[script::bindings::constructor]]
    static sp<Integer> create(const sp<Integer>& value);
//[[script::bindings::constructor]]
    static sp<Integer> create(int32_t value);

//[[script::bindings::operator(+)]]
    static sp<Integer> add(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(+=)]]
    static void iadd(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Integer> sub(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(-=)]]
    static void isub(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Integer> mul(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(%)]]
    static sp<Integer> mod(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(*=)]]
    static void imul(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Integer> floordiv(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Integer> negative(const sp<Integer>& self);
//[[script::bindings::operator(int)]]
    static int32_t toInt32(const sp<Integer>& self);
//[[script::bindings::operator(float)]]
    static float toFloat(const sp<Integer>& self);

//[[script::bindings::property]]
    static int32_t val(const sp<Integer>& self);
//[[script::bindings::property]]
    static void setVal(const sp<Integer::Impl>& self, int32_t value);
//[[script::bindings::property]]
    static void setVal(const sp<IntegerWrapper>& self, int32_t value);
//[[script::bindings::property]]
    static const sp<Integer>& delegate(const sp<Integer>& self);
//[[script::bindings::property]]
    static void setDelegate(const sp<Integer>& self, const sp<Integer>& delegate);

//[[script::bindings::classmethod]]
    static void set(const sp<Integer::Impl>& self, int32_t value);
//[[script::bindings::classmethod]]
    static void set(const sp<IntegerWrapper>& self, int32_t value);
//[[script::bindings::classmethod]]
    static void set(const sp<IntegerWrapper>& self, const sp<Integer>& delegate);

//[[script::bindings::classmethod]]
    static void fix(const sp<Integer>& self);


//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Integer> {
    public:
        DICTIONARY(BeanFactory& beanFactory, const String& value);

        virtual sp<Integer> build(const sp<Scope>& args) override;

    private:
        int32_t _value;
    };
};

}

#endif