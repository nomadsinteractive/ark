#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"
#include "graphics/impl/vec/vec4_impl.h"
#include "graphics/util/vec_type.h"

namespace ark {

//[[script::bindings::class("Vec4")]]
class ARK_API Vec4Type final : public VecType<V4, Vec4Impl> {
public:
//  [[script::bindings::constructor]]
    static sp<Vec4> create(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z, sp<Numeric> w);
//  [[script::bindings::constructor]]
    static sp<Vec4> create(float x, float y, float z, float w);

/**
//  [[script::bindings::operator(+)]]
    static sp<Vec4> add(sp<Vec4> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Vec4> sub(sp<Vec4> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(sp<Vec4> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(sp<Numeric> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(sp<Vec4> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec4> truediv(sp<Vec4> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec4> truediv(sp<Vec4> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec4> floordiv(sp<Vec4> lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec4> floordiv(sp<Vec4> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(neg)]]
    static sp<Vec4> negative(sp<Vec4> self);
//  [[script::bindings::operator(abs)]]
    static sp<Vec4> absolute(sp<Vec4> self);

//  [[script::bindings::classmethod]]
    static sp<Vec4> normalize(sp<Vec4> self);
//  [[script::bindings::classmethod]]
    static sp<Vec4> integral(sp<Vec4> self, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> integralS2(sp<Vec4> self, const V4& s0, const Optional<V4>& s1 = Optional<V4>(), sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> distance(sp<Vec4> self, sp<Vec4> other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> distance2(sp<Vec4> self, sp<Vec4> other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> hypot(sp<Vec4> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> hypot2(sp<Vec4> self);

//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V4>>& self, sp<Vec4> val);
//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V4>>& self, const V4& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<Vec4>& self, const V4& val);

//  [[script::bindings::property]]
    static V4 val(const sp<Vec4>& self);

//  [[script::bindings::property]]
    static sp<Numeric> x(const sp<Vec4>& self);
//  [[script::bindings::property]]
    static void setX(const sp<Vec4>& self, float x);
//  [[script::bindings::property]]
    static void setX(const sp<Vec4>& self, sp<Numeric> x);
//  [[script::bindings::property]]
    static sp<Numeric> y(const sp<Vec4>& self);
//  [[script::bindings::property]]
    static void setY(const sp<Vec4>& self, float y);
//  [[script::bindings::property]]
    static void setY(const sp<Vec4>& self, sp<Numeric> y);
//  [[script::bindings::property]]
    static sp<Numeric> z(const sp<Vec4>& self);
//  [[script::bindings::property]]
    static void setZ(const sp<Vec4>& self, float z);
//  [[script::bindings::property]]
    static void setZ(const sp<Vec4>& self, sp<Numeric> z);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<Vec4>& self);
//  [[script::bindings::seq(get)]]
    static Optional<float> getItem(const sp<Vec4>& self, ptrdiff_t index);
**/
//  [[script::bindings::property]]
    static sp<Numeric> w(const sp<Vec4>& self);
//  [[script::bindings::property]]
    static void setW(const sp<Vec4>& self, float w);
//  [[script::bindings::property]]
    static void setW(const sp<Vec4>& self, sp<Numeric> w);

//  [[script::bindings::property]]
    static sp<Vec4> xyzw(sp<Vec4> self);
/**

//  [[script::bindings::property]]
    static sp<Vec2> xy(sp<Vec4> self);
//  [[script::bindings::property]]
    static sp<Vec3> xyz(const sp<Vec4>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec4> freeze(sp<Vec4> self);

//  [[script::bindings::classmethod]]
    static sp<Size> toSize(const sp<Vec4>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec4> wrap(sp<Vec4> self);
//  [[script::bindings::classmethod]]
    static sp<Vec4> synchronize(sp<Vec4> self, sp<Boolean> canceled = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec4> floor(sp<Vec4> self, sp<Numeric> mod = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> floor(sp<Vec4> self, sp<Vec4> mod = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> ceil(sp<Vec4> self, sp<Numeric> mod = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> ceil(sp<Vec4> self, sp<Vec4> mod = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> round(sp<Vec4> self);

//  [[script::bindings::classmethod]]
    static sp<Vec4> lerp(sp<Vec4> self, sp<Vec4> b, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> track(sp<Vec4> self, V4 s0, float speed, float snapDistance2, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> sod(sp<Vec4> self, V4 s0, float f, float z = 1.0f, float r = 0, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec4> dye(sp<Vec4> self, sp<Boolean> c = nullptr, String message = "");

//  [[script::bindings::classmethod]]
    static sp<Vec4> ifElse(sp<Vec4> self, sp<Boolean> condition, sp<Vec4> otherwise);

//  [[script::bindings::property]]
    static sp<Vec4> wrapped(const sp<Vec4>& self);
**/
};

}
