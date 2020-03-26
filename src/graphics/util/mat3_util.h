#ifndef ARK_GRAPHICS_UTIL_MAT3_UTIL_H_
#define ARK_GRAPHICS_UTIL_MAT3_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::class("Mat3")]]
class ARK_API Mat3Util final {
public:
//[[script::bindings::constructor]]
    static sp<Mat3> create(const V3& t, const V3& b, const V3& n);
//[[script::bindings::constructor]]
    static sp<Mat3> create(const sp<Vec3>& t = nullptr, const sp<Vec3>& b = nullptr, const sp<Vec3>& n = nullptr);

//[[script::bindings::operator(*)]]
    static sp<Mat3> mul(const sp<Mat3>& lvalue, const sp<Mat3>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Mat3>& lvalue, const sp<Vec3>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Mat3>& lvalue, const V3& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Mat3>& lvalue, const sp<Vec2>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Mat3>& lvalue, const V2& rvalue);

//[[script::bindings::auto]]
    static sp<Mat3> identity();

//[[script::bindings::classmethod]]
    static void fix(const sp<Mat3>& self);

private:
    static sp<Mat3Impl> ensureImpl(const sp<Mat3>& self);

};

}

#endif
