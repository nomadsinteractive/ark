#ifndef ARK_GRAPHICS_UTIL_VEC2_UTIL_H_
#define ARK_GRAPHICS_UTIL_VEC2_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

namespace ark {

//[[script::bindings::class("Vec2")]]
class ARK_API Vec2Util final {
public:
//[[script::bindings::constructor]]
    static sp<Vec2> create(const sp<Numeric>& x, const sp<Numeric>& y);
//[[script::bindings::constructor]]
    static sp<Vec2> create(float x, float y);

//[[script::bindings::operator(+)]]
    static sp<Vec2> add(const sp<Vec2>& self, const sp<Vec2>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Vec2> sub(const sp<Vec2>& self, const sp<Vec2>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Vec2>& self, const sp<Vec2>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Vec2> truediv(const sp<Vec2>& self, const sp<Vec2>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Vec2> floordiv(const sp<Vec2>& self, const sp<Vec2>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Vec2> negative(const sp<Vec2>& self);

//[[script::bindings::classmethod]]
    static sp<Vec2> transform(const sp<Vec2>& self, const sp<Transform>& transform, const sp<Vec2>& org);

//[[script::bindings::property]]
    static V2 val(const sp<Vec2>& self);
//[[script::bindings::property]]
    static V2 xy(const sp<Vec2>& self);
//[[script::bindings::property]]
    static void setXy(const sp<Vec2>& self, const V2& xy);

//[[script::bindings::property]]
    static float x(const sp<Vec2>& self);
//[[script::bindings::property]]
    static void setX(const sp<Vec2>& self, float x);
//[[script::bindings::property]]
    static void setX(const sp<Vec2>& self, const sp<Numeric>& x);
//[[script::bindings::property]]
    static float y(const sp<Vec2>& self);
//[[script::bindings::property]]
    static void setY(const sp<Vec2>& self, float y);
//[[script::bindings::property]]
    static void setY(const sp<Vec2>& self, const sp<Numeric>& y);
//[[script::bindings::property]]
    static sp<Numeric> vx(const sp<Vec2>& self);
//[[script::bindings::property]]
    static void setVx(const sp<Vec2>& self, const sp<Numeric>& x);
//[[script::bindings::property]]
    static sp<Numeric> vy(const sp<Vec2>& self);
//[[script::bindings::property]]
    static void setVy(const sp<Vec2>& self, const sp<Numeric>& y);

//[[script::bindings::classmethod]]
    static void fix(const sp<Vec2>& self);

private:
    static sp<Vec2Impl> ensureImpl(const sp<Vec2>& self);

};

}

#endif
