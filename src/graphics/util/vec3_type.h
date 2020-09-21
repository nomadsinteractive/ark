#ifndef ARK_GRAPHICS_UTIL_VEC3_TYPE_H_
#define ARK_GRAPHICS_UTIL_VEC3_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::class("Vec3")]]
class ARK_API Vec3Type final {
public:
//[[script::bindings::constructor]]
    static sp<Vec3> create(float x, float y, float z);
//[[script::bindings::constructor]]
    static sp<Vec3> create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z);
    static sp<Vec3> create(const sp<Vec2>& vec2);

//[[script::bindings::operator(+)]]
    static sp<Vec3> add(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Vec3> sub(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, float rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& lvalue, sp<Numeric>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& lvalue, const sp<Vec3>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Vec3> floordiv(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Vec3> negative(const sp<Vec3>& self);

//[[script::bindings::classmethod]]
    static void set(const sp<Vec3>& self, const V3& val);

//[[script::bindings::property]]
    static V3 xyz(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setXyz(const sp<Vec3>& self, const V3& xyz);
//[[script::bindings::property]]
    static V2 xy(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setXy(const sp<Vec3>& self, const V2& xy);

//[[script::bindings::property]]
    static float x(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setX(const sp<Vec3>& self, float x);
//[[script::bindings::property]]
    static void setX(const sp<Vec3>& self, const sp<Numeric>& x);
//[[script::bindings::property]]
    static float y(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setY(const sp<Vec3>& self, float y);
//[[script::bindings::property]]
    static void setY(const sp<Vec3>& self, const sp<Numeric>& y);
//[[script::bindings::property]]
    static float z(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setZ(const sp<Vec3>& self, float z);
//[[script::bindings::property]]
    static void setZ(const sp<Vec3>& self, const sp<Numeric>& z);
//[[script::bindings::property]]
    static sp<Numeric> vx(const sp<Vec3>& self);
//[[script::bindings::property]]
    static sp<Numeric> vy(const sp<Vec3>& self);
//[[script::bindings::property]]
    static sp<Numeric> vz(const sp<Vec3>& self);

//[[script::bindings::classmethod]]
    static void fix(const sp<Vec3>& self);
//[[script::bindings::classmethod]]
    static sp<Vec3> freeze(const sp<Vec3>& self);

//[[script::bindings::classmethod]]
    static sp<Vec3> cross(const sp<Vec3>& self, const sp<Vec3>& other);
//[[script::bindings::classmethod]]
    static sp<Vec3> cross(const sp<Vec3>& self, const V3& other);
//[[script::bindings::classmethod]]
    static sp<Vec3> normalize(const sp<Vec3>& self);
//[[script::bindings::classmethod]]
    static sp<Vec3> integral(const sp<Vec3>& self, const sp<Numeric>& t = nullptr);

//[[script::bindings::classmethod]]
    static sp<Numeric> distanceTo(const sp<Vec3>& self, const sp<Vec3>& other);

private:
    static sp<Vec3Impl> ensureImpl(const sp<Vec3>& self);

};

}

#endif
