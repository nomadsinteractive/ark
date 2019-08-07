#ifndef ARK_GRAPHICS_UTIL_VEC4_UTIL_H_
#define ARK_GRAPHICS_UTIL_VEC4_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::class("Vec4")]]
class ARK_API Vec4Util final {
public:
//[[script::bindings::constructor]]
    static sp<Vec4> create(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w);
//[[script::bindings::constructor]]
    static sp<Vec4> create(float x, float y, float z, float w);

//[[script::bindings::operator(+)]]
    static sp<Vec4> add(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Vec4> sub(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec4> mul(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Vec4> truediv(const sp<Vec4>& lvalue, const sp<Vec4>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Vec4> floordiv(const sp<Vec4>& self, const sp<Vec4>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Vec4> negative(const sp<Vec4>& self);

//[[script::bindings::classmethod]]
    static sp<Vec4> transform(const sp<Vec4>& self, const sp<Transform>& transform, const sp<Vec4>& org);

//[[script::bindings::classmethod]]
    static void set(const sp<Vec4>& self, const V4& val);

//[[script::bindings::property]]
    static V3 xyz(const sp<Vec4>& self);
//[[script::bindings::property]]
    static V2 xy(const sp<Vec4>& self);
//[[script::bindings::property]]
    static void setXy(const sp<Vec4>& self, const V2& xy);

//[[script::bindings::property]]
    static float x(const sp<Vec4>& self);
//[[script::bindings::property]]
    static void setX(const sp<Vec4>& self, float x);
//[[script::bindings::property]]
    static void setX(const sp<Vec4>& self, const sp<Numeric>& x);
//[[script::bindings::property]]
    static float y(const sp<Vec4>& self);
//[[script::bindings::property]]
    static void setY(const sp<Vec4>& self, float y);
//[[script::bindings::property]]
    static void setY(const sp<Vec4>& self, const sp<Numeric>& y);
//[[script::bindings::property]]
    static float z(const sp<Vec4>& self);
//[[script::bindings::property]]
    static void setZ(const sp<Vec4>& self, float z);
//[[script::bindings::property]]
    static void setZ(const sp<Vec4>& self, const sp<Numeric>& z);
//[[script::bindings::property]]
    static float w(const sp<Vec4>& self);
//[[script::bindings::property]]
    static void setW(const sp<Vec4>& self, float w);
//[[script::bindings::property]]
    static void setW(const sp<Vec4>& self, const sp<Numeric>& w);
//[[script::bindings::property]]
    static sp<Numeric> vx(const sp<Vec4>& self);
//[[script::bindings::property]]
    static sp<Numeric> vy(const sp<Vec4>& self);
//[[script::bindings::property]]
    static sp<Numeric> vz(const sp<Vec4>& self);
//[[script::bindings::property]]
    static sp<Numeric> vw(const sp<Vec4>& self);

//[[script::bindings::classmethod]]
    static void fix(const sp<Vec4>& self);

private:
    static sp<Vec4Impl> ensureImpl(const sp<Vec4>& self);

};

}

#endif
