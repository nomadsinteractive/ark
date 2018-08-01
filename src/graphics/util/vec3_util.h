#ifndef ARK_GRAPHICS_UTIL_VEC3_UTIL_H_
#define ARK_GRAPHICS_UTIL_VEC3_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::class("Vec3")]]
class ARK_API Vec3Util final {
public:
//[[script::bindings::constructor]]
    static sp<Vec3> create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z);
//[[script::bindings::constructor]]
    static sp<Vec3> create(float x, float y, float z);

//[[script::bindings::operator(+)]]
    static sp<Vec3> add(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Vec3> sub(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Vec3> truediv(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Vec3> floordiv(const sp<Vec3>& self, const sp<Vec3>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Vec3> negative(const sp<Vec3>& self);

//[[script::bindings::classmethod]]
    static sp<Vec3> transform(const sp<Vec3>& self, const sp<Transform>& transform, const sp<Vec3>& org);

//[[script::bindings::property]]
    static V3 val(const sp<Vec3>& self);
//[[script::bindings::property]]
    static V2 xy(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setXy(const sp<Vec3>& self, const V2& xy);

//[[script::bindings::property]]
    static float x(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setX(const sp<Vec3>& self, float x);
//[[script::bindings::property]]
    static float y(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setY(const sp<Vec3>& self, float y);
//[[script::bindings::property]]
    static sp<Numeric> vx(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setVx(const sp<Vec3>& self, const sp<Numeric>& x);
//[[script::bindings::property]]
    static sp<Numeric> vy(const sp<Vec3>& self);
//[[script::bindings::property]]
    static void setVy(const sp<Vec3>& self, const sp<Numeric>& y);

//[[script::bindings::classmethod]]
    static void fix(const sp<Vec3>& self);

private:
    static sp<Vec3Impl> ensureImpl(const sp<Vec3>& self);

};

}

#endif
