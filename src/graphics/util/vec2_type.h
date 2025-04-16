#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"
#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/util/vec_type.h"

namespace ark {

//[[script::bindings::class("Vec2")]]
class ARK_API Vec2Type final : public VecType<V2, Vec2Impl> {
public:
//  [[script::bindings::constructor]]
    static sp<Vec2> create(sp<Numeric> x, sp<Numeric> y = nullptr);
//  [[script::bindings::constructor]]
    static sp<Vec2> create(float x, float y);

/**
//  [[script::bindings::operator(+)]]
    static sp<Vec2> add(sp<Vec2> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Vec2> sub(sp<Vec2> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(sp<Vec2> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(sp<Vec2> lvalue, float rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(sp<Numeric> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(sp<Vec2> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(sp<Vec2> lvalue, float rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(sp<Vec2> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(sp<Vec2> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(sp<Vec2> lvalue, const V2& rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec2> floordiv(sp<Vec2> lvalue, float rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec2> floordiv(sp<Vec2> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec2> floordiv(sp<Vec2> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(neg)]]
    static sp<Vec2> negative(sp<Vec2> self);
//  [[script::bindings::operator(abs)]]
    static sp<Vec2> absolute(sp<Vec2> self);

//  [[script::bindings::classmethod]]
    static sp<Vec2> normalize(sp<Vec2> self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> integral(sp<Vec2> self, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec2> integralS2(sp<Vec2> self, const V2& s0, const Optional<V2>& s1 = Optional<V2>(), sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> distance(sp<Vec2> self, sp<Vec2> other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> distance2(sp<Vec2> self, sp<Vec2> other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> hypot(sp<Vec2> self);

//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V2>>& self, sp<Vec2> val);
//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V2>>& self, const V2& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<Vec2>& self, const V2 val);

//  [[script::bindings::property]]
    static V2 val(const sp<Vec2>& self);

//  [[script::bindings::property]]
    static sp<Numeric> x(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setX(const sp<Vec2>& self, float x);
//  [[script::bindings::property]]
    static void setX(const sp<Vec2>& self, sp<Numeric> x);
//  [[script::bindings::property]]
    static sp<Numeric> y(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setY(const sp<Vec2>& self, float y);
//  [[script::bindings::property]]
    static void setY(const sp<Vec2>& self, sp<Numeric> y);

//  [[script::bindings::classmethod]]
    static sp<Size> toSize(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static sp<Vec2> xy(sp<Vec2> self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<Vec2>& self);
//  [[script::bindings::seq(get)]]
    static Optional<float> getItem(const sp<Vec2>& self, ptrdiff_t index);

//  [[script::bindings::classmethod]]
    static sp<Vec2> freeze(sp<Vec2> self);

//  [[script::bindings::classmethod]]
    static sp<Vec2> wrap(sp<Vec2> self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> synchronize(sp<Vec2> self, sp<Boolean> discarded = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec2> modFloor(sp<Vec2> self, sp<Numeric> mod);
//  [[script::bindings::classmethod]]
    static sp<Vec2> modFloor(sp<Vec2> self, sp<Vec2> mod);

//  [[script::bindings::classmethod]]
    static sp<Vec2> floor(sp<Vec2> self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> ceil(sp<Vec2> self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> round(sp<Vec2> self);

//  [[script::bindings::classmethod]]
    static sp<Vec2> lerp(sp<Vec2> self, sp<Vec2> b, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec2> sod(sp<Vec2> self, const V2& s0, float f, float z = 1.0f, float r = 0, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec2> dye(sp<Vec2> self, sp<Boolean> c = nullptr, String message = "");

//  [[script::bindings::classmethod]]
    static sp<Vec2> ifElse(sp<Vec2> self, sp<Boolean> condition, sp<Vec2> otherwise);

//  [[script::bindings::property]]
    static sp<Vec2> wrapped(const sp<Vec2>& self);
**/
//  [[script::bindings::classmethod]]
    static sp<Vec3> extend(sp<Vec2> self, sp<Numeric> z);
//  [[script::bindings::classmethod]]
    static sp<Vec4> extend(sp<Vec2> self, sp<Vec2> z);
//  [[script::bindings::classmethod]]
    static sp<Vec2> fence(sp<Vec2> self, sp<Vec3> plane, sp<Observer> observer);
//  [[script::bindings::classmethod]]
    static sp<Numeric> atan2(sp<Vec2> self);
};

}
