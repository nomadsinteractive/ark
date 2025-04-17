#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/util/vec_type.h"

namespace ark {

//[[script::bindings::class("Vec3")]]
class ARK_API Vec3Type final : public VecType<V3, Vec3Impl> {
public:
//  [[script::bindings::constructor]]
    static sp<Vec3> create(sp<Numeric> x, sp<Numeric> y = nullptr, sp<Numeric> z = nullptr);
//  [[script::bindings::constructor]]
    static sp<Vec3> create(float x, float y, float z);

/**
//  [[script::bindings::operator(+)]]
    static sp<Vec3> add(const sp<Vec3>& lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Vec3> sub(const sp<Vec3>& lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, float rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& lvalue, sp<Numeric> rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec3> floordiv(const sp<Vec3>& self, sp<Vec3> rvalue);
//  [[script::bindings::operator(neg)]]
    static sp<Vec3> negative(const sp<Vec3>& self);
//  [[script::bindings::operator(abs)]]
    static sp<Vec3> absolute(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V3>>& self, sp<Vec3> val);
//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V3>>& self, const V3& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<Vec3>& self, const V3& val);

//  [[script::bindings::property]]
    static V3 val(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static sp<Vec3> xyz(sp<Vec3> self);
//  [[script::bindings::property]]
    static sp<Vec2> xy(sp<Vec3> self);

//  [[script::bindings::property]]
    static sp<Numeric> x(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setX(const sp<Vec3>& self, float x);
//  [[script::bindings::property]]
    static void setX(const sp<Vec3>& self, sp<Numeric> x);
//  [[script::bindings::property]]
    static sp<Numeric> y(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setY(const sp<Vec3>& self, float y);
//  [[script::bindings::property]]
    static void setY(const sp<Vec3>& self, sp<Numeric> y);
//  [[script::bindings::property]]
    static sp<Numeric> z(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setZ(const sp<Vec3>& self, float z);
//  [[script::bindings::property]]
    static void setZ(const sp<Vec3>& self, sp<Numeric> z);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<Vec3>& self);
//  [[script::bindings::seq(get)]]
    static Optional<float> getItem(const sp<Vec3>& self, ptrdiff_t idx);

//  [[script::bindings::classmethod]]
    static sp<Size> toSize(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec3> freeze(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec3> wrap(const sp<Vec3>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> update(sp<Vec3> self, sp<Boolean> canceled = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec3> synchronize(sp<Vec3> self, sp<Boolean> canceled = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec3> modFloor(sp<Vec3> self, sp<Numeric> mod);
//  [[script::bindings::classmethod]]
    static sp<Vec3> modFloor(sp<Vec3> self, sp<Vec3> mod);

//  [[script::bindings::classmethod]]
    static sp<Vec3> floor(sp<Vec3> self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> ceil(sp<Vec3> self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> round(sp<Vec3> self);

//  [[script::bindings::classmethod]]
    static sp<Vec3> ifElse(sp<Vec3> self, sp<Boolean> condition, sp<Vec3> otherwise);
//  [[script::bindings::classmethod]]
    static sp<Vec3> lerp(const sp<Vec3>& self, const sp<Vec3>& b, sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec3> sod(sp<Vec3> self, const V3& s0, float f, float z = 1.0f, float r = 0, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec3> dye(sp<Vec3> self, sp<Boolean> c = nullptr, String message = "");

//  [[script::bindings::classmethod]]
    static sp<Vec3> normalize(const sp<Vec3>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> integral(const sp<Vec3>& self, sp<Numeric> t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec3> integralS2(sp<Vec3> self, const V3& s0, const Optional<V3>& s1 = Optional<V3>(), sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> distance(const sp<Vec3>& self, const sp<Vec3>& other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> distance2(const sp<Vec3>& self, const sp<Vec3>& other);
//  [[script::bindings::classmethod]]
    static sp<Numeric> hypot(sp<Vec3> self);
**/
//  [[script::bindings::classmethod]]
    static sp<Vec4> extend(sp<Vec3> self, sp<Numeric> w);
//  [[script::bindings::classmethod]]
    static sp<Vec3> cross(sp<Vec3> self, sp<Vec3> other);
//  [[script::bindings::classmethod]]
    static sp<Vec3> cross(sp<Vec3> self, const V3& other);
};

}
