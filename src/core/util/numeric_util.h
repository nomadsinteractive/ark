#ifndef ARK_CORE_UTIL_NUMERIC_UTIL_H_
#define ARK_CORE_UTIL_NUMERIC_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Numeric")]]
class ARK_API NumericUtil final {
public:
//[[script::bindings::constructor]]
    static sp<Numeric> create(const sp<Numeric>& value);
//[[script::bindings::constructor]]
    static sp<Numeric> create(float value);

//[[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& lvalue, float rvalue);
//[[script::bindings::operator(+)]]
    static sp<Numeric> add(float lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Numeric> sub(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Numeric> sub(const sp<Numeric>& lvalue, float rvalue);
//[[script::bindings::operator(-)]]
    static sp<Numeric> sub(float lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Numeric> mul(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Numeric> mul(const sp<Numeric>& lvalue, float rvalue);
//[[script::bindings::operator(*)]]
    static sp<Numeric> mul(float lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Numeric>& lvalue, float rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(float lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(%)]]
    static sp<Numeric> mod(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(%)]]
    static sp<Numeric> mod(const sp<Numeric>& lvalue, float rvalue);
//[[script::bindings::operator(%)]]
    static sp<Numeric> mod(float lvalue, const sp<Numeric>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Numeric> negative(const sp<Numeric>& self);
//[[script::bindings::operator(pow)]]
    static sp<Numeric> pow(const sp<Numeric>& x, const sp<Integer>& y, const sp<Integer>& z = nullptr);
//[[script::bindings::operator(pow)]]
    static sp<Numeric> pow(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Integer>& z = nullptr);
//[[script::bindings::operator(int)]]
    static int32_t toInt32(const sp<Numeric>& self);
//[[script::bindings::operator(float)]]
    static float toFloat(const sp<Numeric>& self);

//[[script::bindings::operator(>)]]
    static sp<Boolean> gt(const sp<Numeric>& self, const sp<Numeric>& other);
//[[script::bindings::operator(>=)]]
    static sp<Boolean> ge(const sp<Numeric>& self, const sp<Numeric>& other);
//[[script::bindings::operator(<)]]
    static sp<Boolean> lt(const sp<Numeric>& self, const sp<Numeric>& other);
//[[script::bindings::operator(<=)]]
    static sp<Boolean> le(const sp<Numeric>& self, const sp<Numeric>& other);
//[[script::bindings::operator(==)]]
    static sp<Boolean> eq(const sp<Numeric>& self, const sp<Numeric>& other);
//[[script::bindings::operator(!=)]]
    static sp<Boolean> ne(const sp<Numeric>& self, const sp<Numeric>& other);

//[[script::bindings::property]]
    static float val(const sp<Numeric>& self);
//[[script::bindings::property]]
    static void setVal(const sp<Numeric>& self, float value);
//[[script::bindings::property]]
    static const sp<Numeric>& delegate(const sp<Numeric>& self);
//[[script::bindings::property]]
    static void setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate);

//[[script::bindings::classmethod]]
    static void set(const sp<Numeric::Impl>& self, float value);
//[[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, float value);
//[[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, const sp<Numeric>& delegate);

//[[script::bindings::classmethod]]
    static float fix(const sp<Numeric>& self);
//[[script::bindings::classmethod]]
    static sp<Numeric> wrap(const sp<Numeric>& self);

//[[script::bindings::classmethod]]
    static sp<Expectation> approach(const sp<Numeric>& self, const sp<Numeric>& a1);
//[[script::bindings::classmethod]]
    static sp<Expectation> atLeast(const sp<Numeric>& self, const sp<Numeric>& a1);
//[[script::bindings::classmethod]]
    static sp<Expectation> atMost(const sp<Numeric>& self, const sp<Numeric>& a1);
//[[script::bindings::classmethod]]
    static sp<Expectation> boundary(const sp<Numeric>& self, const sp<Numeric>& a1);
//[[script::bindings::classmethod]]
    static sp<Expectation> clamp(const sp<Numeric>& self, const sp<Numeric>& min, const sp<Numeric>& max);
//[[script::bindings::classmethod]]
    static sp<Expectation> fence(const sp<Numeric>& self, const sp<Numeric>& a1);
//[[script::bindings::classmethod]]
    static sp<Numeric> integral(const sp<Numeric>& self, const sp<Numeric>& t = nullptr);

//[[script::bindings::auto]]
    static sp<Numeric> pursue(float s0, const sp<Numeric>& target, float duration, const sp<Numeric>& t = nullptr);
//[[script::bindings::auto]]
    static sp<Numeric> vibrate(float s0, float v0, float s1, float v1, float duration, const sp<Numeric>& t = nullptr);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Numeric> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

};

}

#endif
