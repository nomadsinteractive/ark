#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Numeric")]]
class ARK_API NumericType final {
public:
//  [[script::bindings::constructor]]
    static sp<NumericWrapper> create(sp<Numeric> value);
//  [[script::bindings::constructor]]
    static sp<NumericWrapper> create(sp<Integer> value);
//  [[script::bindings::constructor]]
    static sp<NumericWrapper> create(float value);

//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& lvalue, float rvalue);
//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(float lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(sp<Numeric> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(sp<Numeric> lvalue, float rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(float lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(sp<Numeric> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(sp<Numeric> lvalue, float rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(sp<Numeric> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(sp<Numeric> lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(sp<Numeric> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Numeric> truediv(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(sp<Numeric> lhs, sp<Integer> rhs);
//  [[script::bindings::operator(%)]]
    static sp<Numeric> mod(sp<Numeric> lhs, sp<Numeric> rhs);
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
    static sp<Numeric> delegate(const sp<Numeric>& self);
//  [[script::bindings::property]]
    static void setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate);

//  [[script::bindings::property]]
    static sp<Observer> observer(const sp<Numeric>& self);

//  [[script::bindings::classmethod]]
    static void set(const sp<Numeric::Impl>& self, float value);
//  [[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, float value);
//  [[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, sp<Numeric> value);

//  [[script::bindings::classmethod]]
    static float fix(const sp<Numeric>& self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> freeze(const sp<Numeric>& self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> wrap(sp<Numeric> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> synchronize(sp<Numeric> self, sp<Boolean> discarded = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> atLeast(sp<Numeric> self, sp<Numeric> a1, sp<Observer> observer = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> atMost(sp<Numeric> self, sp<Numeric> a1, sp<Observer> observer = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> clamp(sp<Numeric> self, sp<Numeric> min, sp<Numeric> max, sp<Observer> observer = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> fence(sp<Numeric> self, sp<Numeric> a1, sp<Observer> observer = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> ifElse(sp<Numeric> self, sp<Boolean> condition, sp<Numeric> negative);
//  [[script::bindings::classmethod]]
    static sp<Numeric> integral(const sp<Numeric>& self, const sp<Numeric>& t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> modFloor(const sp<Numeric>& self, const sp<Numeric>& mod);
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
    class DICTIONARY final : public Builder<Numeric> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

//  [[plugin::builder]]
    class BUILDER final : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

};

}
