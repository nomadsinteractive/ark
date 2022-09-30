#ifndef ARK_GRAPHICS_UTIL_VEC3_TYPE_H_
#define ARK_GRAPHICS_UTIL_VEC3_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

//  [[script::bindings::class("Vec3")]]
class ARK_API Vec3Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Vec3> create(float x, float y, float z);
//  [[script::bindings::constructor]]
    static sp<Vec3> create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z);

//  [[script::bindings::operator(+)]]
    static sp<Vec3> add(const sp<Vec3>& lvalue, const V3& rvalue);
//  [[script::bindings::operator(+)]]
    static sp<Vec3> add(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Vec3> sub(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, const V3& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, float rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, sp<Numeric>& rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& lvalue, const V3& rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(//)]]
    static sp<Vec3> floordiv(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(neg)]]
    static sp<Vec3> negative(const sp<Vec3>& self);
//  [[script::bindings::operator(abs)]]
    static sp<Vec3> absolute(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V3>>& self, const V3& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V3>>& self, const sp<Vec3>& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<Vec3>& self, const V3& val);

//  [[script::bindings::property]]
    static V3 val(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static V3 xyz(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setXyz(const sp<Vec3>& self, const V3& xyz);
//  [[script::bindings::property]]
    static V2 xy(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setXy(const sp<Vec3>& self, const V2& xy);

//  [[script::bindings::property]]
    static float x(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setX(const sp<Vec3>& self, float x);
//  [[script::bindings::property]]
    static void setX(const sp<Vec3>& self, const sp<Numeric>& x);
//  [[script::bindings::property]]
    static float y(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setY(const sp<Vec3>& self, float y);
//  [[script::bindings::property]]
    static void setY(const sp<Vec3>& self, const sp<Numeric>& y);
//  [[script::bindings::property]]
    static float z(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static void setZ(const sp<Vec3>& self, float z);
//  [[script::bindings::property]]
    static void setZ(const sp<Vec3>& self, const sp<Numeric>& z);
//  [[script::bindings::property]]
    static sp<Numeric> vx(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static sp<Numeric> vy(const sp<Vec3>& self);
//  [[script::bindings::property]]
    static sp<Numeric> vz(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec4> extend(sp<Vec3> self, sp<Numeric> w);

//  [[script::bindings::classmethod]]
    static void fix(const sp<Vec3>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> freeze(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static sp<Size> toSize(const sp<Vec3>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec3> wrap(const sp<Vec3>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> synchronize(const sp<Vec3>& self, const sp<Boolean>& disposed = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec3> modFloor(const sp<Vec3>& self, const sp<Numeric>& mod);
//  [[script::bindings::classmethod]]
    static sp<Vec3> modFloor(const sp<Vec3>& self, const sp<Vec3>& mod);
//  [[script::bindings::classmethod]]
    static sp<Vec3> modCeil(const sp<Vec3>& self, const sp<Numeric>& mod);
//  [[script::bindings::classmethod]]
    static sp<Vec3> modCeil(const sp<Vec3>& self, const sp<Vec3>& mod);

//  [[script::bindings::classmethod]]
    static sp<Vec3> ifElse(const sp<Vec3>& self, const sp<Boolean>& condition, const sp<Vec3>& otherwise);

//  [[script::bindings::classmethod]]
    static sp<Vec3> attract(const sp<Vec3>& self, const V3& s0, float duration, const sp<Numeric>& t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec3> lerp(const sp<Vec3>& self, const sp<Vec3>& b, const sp<Numeric>& t);

//  [[script::bindings::classmethod]]
    static sp<Vec3> sod(sp<Vec3> self, float k, float z, float r, sp<Numeric> t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec3> cross(const sp<Vec3>& self, const sp<Vec3>& other);
//  [[script::bindings::classmethod]]
    static sp<Vec3> cross(const sp<Vec3>& self, const V3& other);
//  [[script::bindings::classmethod]]
    static sp<Vec3> normalize(const sp<Vec3>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> integral(const sp<Vec3>& self, const sp<Numeric>& t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Numeric> distanceTo(const sp<Vec3>& self, const sp<Vec3>& other);

private:
    static sp<Vec3Impl> ensureImpl(const sp<Vec3>& self);

};

}

#endif
