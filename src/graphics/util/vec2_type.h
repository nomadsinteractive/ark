#ifndef ARK_GRAPHICS_UTIL_VEC2_TYPE_H_
#define ARK_GRAPHICS_UTIL_VEC2_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

namespace ark {

//  [[script::bindings::class("Vec2")]]
class ARK_API Vec2Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Vec2> create(float x, float y);
//  [[script::bindings::constructor]]
    static sp<Vec2> create(const sp<Numeric>& x, const sp<Numeric>& y);

//  [[script::bindings::operator(+)]]
    static sp<Vec2> add(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::operator(-)]]
    static sp<Vec2> sub(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Vec2>& lvalue, float rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(float lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Vec2>& lvalue, sp<Numeric>& rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(const sp<Vec2>& lvalue, float rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(const sp<Vec2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::operator(/)]]
    static sp<Vec2> truediv(const sp<Vec2>& lvalue, const sp<Numeric>& rvalue);
//  [[script::bindings::operator(neg)]]
    static sp<Vec2> negative(const sp<Vec2>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec2> transform(const sp<Vec2>& self, const sp<Transform>& transform);
//  [[script::bindings::classmethod]]
    static sp<Vec2> normalize(const sp<Vec2>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> integral(const sp<Vec2>& self, const sp<Numeric>& t = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Vec2> integralWithResistance(const sp<Vec2>& self, const V2& v0, const sp<Numeric>& cd, const sp<Numeric>& t = nullptr);

//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V2>>& self, const V2& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<VariableWrapper<V2>>& self, const sp<Vec2>& val);
//  [[script::bindings::classmethod]]
    static void set(const sp<Vec2>& self, const V2 val);

//  [[script::bindings::property]]
    static V2 val(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static V2 xy(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setXy(const sp<Vec2>& self, const V2& xy);

//  [[script::bindings::property]]
    static float x(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setX(const sp<Vec2>& self, float x);
//  [[script::bindings::property]]
    static void setX(const sp<Vec2>& self, const sp<Numeric>& x);
//  [[script::bindings::property]]
    static float y(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setY(const sp<Vec2>& self, float y);
//  [[script::bindings::property]]
    static void setY(const sp<Vec2>& self, const sp<Numeric>& y);
//  [[script::bindings::property]]
    static sp<Numeric> vx(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setVx(const sp<Vec2>& self, const sp<Numeric>& x);
//  [[script::bindings::property]]
    static sp<Numeric> vy(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setVy(const sp<Vec2>& self, const sp<Numeric>& y);

//  [[script::bindings::classmethod]]
    static void fix(const sp<Vec2>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> freeze(const sp<Vec2>& self);

//  [[script::bindings::classmethod]]
    static sp<Vec2> wrap(const sp<Vec2>& self);
//  [[script::bindings::classmethod]]
    static sp<Vec2> synchronize(const sp<Vec2>& self, const sp<Boolean>& disposed = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec2> modFloor(const sp<Vec2>& self, const sp<Numeric>& mod);

//  [[script::bindings::classmethod]]
    static sp<Vec2> attract(const sp<Vec2>& self, const V2& s0, float duration, const sp<Numeric>& t = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Vec2> fence(const sp<Vec2>& self, const sp<Vec3>& plane, const sp<Observer>& observer);
//  [[script::bindings::classmethod]]
    static sp<Vec2> ifElse(const sp<Vec2>& self, const sp<Boolean>& condition, const sp<Vec2>& negative);

//  [[script::bindings::classmethod]]
    static sp<Numeric> atan2(const sp<Vec2>& self);

//  [[script::bindings::property]]
    static sp<Vec2> delegate(const sp<Vec2>& self);
//  [[script::bindings::property]]
    static void setDelegate(const sp<Vec2>& self, const sp<Vec2>& delegate);

private:
    static sp<Vec2Impl> ensureImpl(const sp<Vec2>& self);

};

}

#endif
