#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

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
    static sp<Numeric> add(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(sp<Numeric> lhs, float rhs);
//  [[script::bindings::operator(+)]]
    static sp<Numeric> add(float lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(sp<Numeric> lhs, float rhs);
//  [[script::bindings::operator(-)]]
    static sp<Numeric> sub(float lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(*)]]
    static sp<Numeric> mul(sp<Numeric> lhs, float rhs);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(sp<Numeric> lhs, sp<Vec2> rhs);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(sp<Numeric> lhs, sp<Vec3> rhs);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(sp<Numeric> lhs, sp<Vec4> rhs);
//  [[script::bindings::operator(/)]]
    static sp<Numeric> truediv(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(sp<Numeric> lhs, sp<Integer> rhs);
//  [[script::bindings::operator(%)]]
    static sp<Numeric> mod(sp<Numeric> lhs, sp<Numeric> rhs);
//  [[script::bindings::operator(neg)]]
    static sp<Numeric> negative(sp<Numeric> self);
//  [[script::bindings::operator(abs)]]
    static sp<Numeric> absolute(sp<Numeric> self);
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
//  [[script::bindings::classmethod]]
    static sp<Boolean> dirty(sp<Numeric> self);

//  [[script::bindings::property]]
    static float val(const sp<Numeric>& self);

//  [[script::bindings::classmethod]]
    static void set(const sp<Numeric::Impl>& self, float value);
//  [[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, float value);
//  [[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, sp<Numeric> value);

//  [[script::bindings::classmethod]]
    static float update(const sp<Numeric>& self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> freeze(const sp<Numeric>& self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> wrap(sp<Numeric> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> synchronize(sp<Numeric> self, sp<Boolean> canceled = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> atLeast(sp<Numeric> self, sp<Numeric> a1, sp<Runnable> callback = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> atMost(sp<Numeric> self, sp<Numeric> a1, sp<Runnable> callback = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> clamp(sp<Numeric> self, sp<Numeric> min, sp<Numeric> max, sp<Runnable> callback = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> fence(sp<Numeric> self, sp<Numeric> a1, sp<Runnable> callback = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> ifElse(sp<Numeric> self, sp<Boolean> condition, sp<Numeric> negative);
//  [[script::bindings::classmethod]]
    static sp<Numeric> integral(const sp<Numeric>& self, const sp<Numeric>& t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> normalize(sp<Numeric> self);

//  [[script::bindings::classmethod]]
    static sp<Numeric> distance(sp<Numeric> self, sp<Numeric> other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> distance2(sp<Numeric> self, sp<Numeric> other);

//  [[script::bindings::classmethod]]
    static sp<Numeric> floor(sp<Numeric> self, sp<Numeric> mod = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> ceil(sp<Numeric> self, sp<Numeric> mod = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> round(sp<Numeric> self);

//  [[script::bindings::classmethod]]
    static sp<Numeric> lerp(const sp<Numeric>& self, const sp<Numeric>& b, const sp<Numeric>& t);

//  [[script::bindings::classmethod]]
    static sp<Numeric> track(sp<Numeric> self, float s0, float speed, float distance, sp<Future> future = nullptr, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Numeric> sod(sp<Numeric> self, float s0, float f, float z = 1.0, float r = 0, sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> dye(sp<Numeric> self, sp<Boolean> condition = nullptr, String message = "");

//  [[script::bindings::operator(str)]]
    static String str(const sp<Numeric>& self);

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
