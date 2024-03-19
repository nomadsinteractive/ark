#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/expectation_f.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Numeric")]]
class ARK_API NumericType final {
public:
//  [[script::bindings::constructor]]
    static sp<NumericWrapper> create(float value);
//  [[script::bindings::constructor]]
    static sp<NumericWrapper> create(sp<Numeric> value);
//  [[script::bindings::constructor]]
    static sp<NumericWrapper> create(sp<Integer> value);

//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& lvalue, float rvalue);
//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(float lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(const sp<Numeric>& lvalue, float rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(float lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(sp<Numeric> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(sp<Numeric> lvalue, float rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(float lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Numeric>& lvalue, float rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Numeric> truediv(float lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(const sp<Numeric>& lvalue, const sp<Integer>& rvalue);
//  [[script::bindings::operator(%)]]
    static sp<Numeric> mod(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(%)]]
    static sp<Numeric> mod(const sp<Numeric>& lvalue, float rvalue);
//  [[script::bindings::operator(%)]]
    static sp<Numeric> mod(float lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(neg)]]
    static sp<Numeric> negative(const sp<Numeric>& self);
//  [[script::bindings::operator(abs)]]
    static sp<Numeric> absolute(const sp<Numeric>& self);
//  [[script::bindings::operator(pow)]]
    static sp<Numeric> pow(const sp<Numeric>& x, const sp<Integer>& y, const sp<Integer>& z = nullptr);
//  [[script::bindings::operator(pow)]]
    static sp<Numeric> pow(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Integer>& z = nullptr);
//  [[script::bindings::operator(int)]]
    static int32_t toInt32(const sp<Numeric>& self);
//  [[script::bindings::operator(float)]]
    static float toFloat(const sp<Numeric>& self);

//  [[script::bindings::operator(>)]]
    static sp<Boolean> gt(sp<Numeric> self, sp<Numeric> other);
//  [[script::bindings::operator(>=)]]
    static sp<Boolean> ge(sp<Numeric> self, sp<Numeric> other);
//  [[script::bindings::operator(<)]]
    static sp<Boolean> lt(sp<Numeric> self, sp<Numeric> other);
//  [[script::bindings::operator(<=)]]
    static sp<Boolean> le(sp<Numeric> self, sp<Numeric> other);
//  [[script::bindings::operator(==)]]
    static sp<Boolean> eq(sp<Numeric> self, sp<Numeric> other);
//  [[script::bindings::operator(!=)]]
    static sp<Boolean> ne(sp<Numeric> self, sp<Numeric> other);

//  [[script::bindings::property]]
    static float val(const sp<Numeric>& self);
//  [[script::bindings::property]]
    static const sp<Numeric>& delegate(const sp<Numeric>& self);
//  [[script::bindings::property]]
    static void setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate);

//  [[script::bindings::classmethod]]
    static void set(const sp<Numeric::Impl>& self, float value);
//  [[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, float value);
//  [[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, const sp<Numeric>& delegate);

//  [[script::bindings::classmethod]]
    static float fix(const sp<Numeric>& self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> freeze(const sp<Numeric>& self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> wrap(sp<Numeric> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> synchronize(const sp<Numeric>& self, const sp<Boolean>& disposed = nullptr);

//  [[script::bindings::classmethod]]
    static sp<ExpectationF> approach(const sp<Numeric>& self, const sp<Numeric>& a1);
//  [[script::bindings::classmethod]]
    static sp<ExpectationF> atLeast(const sp<Numeric>& self, const sp<Numeric>& a1);
//  [[script::bindings::classmethod]]
    static sp<ExpectationF> atMost(const sp<Numeric>& self, const sp<Numeric>& a1);
//  [[script::bindings::classmethod]]
    static sp<ExpectationF> boundary(const sp<Numeric>& self, const sp<Numeric>& a1);
//  [[script::bindings::classmethod]]
    static sp<ExpectationF> clamp(const sp<Numeric>& self, const sp<Numeric>& min, const sp<Numeric>& max);
//  [[script::bindings::classmethod]]
    static sp<ExpectationF> fence(const sp<Numeric>& self, const sp<Numeric>& a1);
//  [[script::bindings::classmethod]]
    static sp<Numeric> ifElse(sp<Numeric> self, sp<Boolean> condition, sp<Numeric> negative);
//  [[script::bindings::classmethod]]
    static sp<Numeric> integral(const sp<Numeric>& self, const sp<Numeric>& t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> integralWithResistance(const sp<Numeric>& self, float v0, const sp<Numeric>& cd, const sp<Numeric>& t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> modFloor(const sp<Numeric>& self, const sp<Numeric>& mod);
//  [[script::bindings::classmethod]]
    static sp<Numeric> modCeil(const sp<Numeric>& self, const sp<Numeric>& mod);
//  [[script::bindings::classmethod]]
    static sp<Numeric> floor(sp<Numeric> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> ceil(sp<Numeric> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> round(sp<Numeric> self);

//  [[script::bindings::classmethod]]
    static sp<Numeric> lerp(const sp<Numeric>& self, const sp<Numeric>& b, const sp<Numeric>& t);

//  [[script::bindings::classmethod]]
    static sp<Numeric> sod(sp<Numeric> self, float d0, float k, float z = 1.0, float r = 0, sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> periodic(const sp<Numeric>& self, const sp<Numeric>& interval = nullptr, const sp<Numeric>& duration = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> dye(sp<Numeric> self, sp<Boolean> condition = nullptr, String message = "");

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Numeric> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        virtual sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

};

}
