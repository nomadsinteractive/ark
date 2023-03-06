#ifndef ARK_GRAPHICS_UTIL_MAT4_TYPE_H_
#define ARK_GRAPHICS_UTIL_MAT4_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

//[[script::bindings::class("Mat4")]]
class ARK_API Mat4Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Mat4> create(const M4& m = M4::identity());
//  [[script::bindings::constructor]]
    static sp<Mat4> create(sp<Mat4> other);
    static sp<Mat4> create(const V4& t, const V4& b, const V4& n, const V4& w);
    static sp<Mat4> create(sp<Vec4> t = nullptr, sp<Vec4> b = nullptr, sp<Vec4> n = nullptr, sp<Vec4> w = nullptr);

//  [[script::bindings::property]]
    static M4 val(const sp<Mat4>& self);

//  [[script::bindings::operator(@)]]
    static sp<Mat4> matmul(sp<Mat4> lvalue, sp<Mat4> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Mat4> matmul(sp<Mat4> lvalue, const M4& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Mat4> matmul(const M4& lvalue, sp<Mat4> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec4> matmul(sp<Mat4> lvalue, sp<Vec4> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec4> matmul(sp<Mat4> lvalue, const V4& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(sp<Mat4> lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(sp<Mat4> lvalue, const V3& rvalue);

//  [[script::bindings::auto]]
    static sp<Mat4> identity();

//  [[script::bindings::classmethod]]
    static sp<Mat4> translate(sp<Mat4> self, sp<Vec3> translation);

//  [[script::bindings::classmethod]]
    static sp<Mat4> freeze(const sp<Mat4>& self);

//  [[script::bindings::classmethod]]
    static sp<Mat4> dye(sp<Mat4> self, sp<Boolean> condition = nullptr, String message = "");

private:
    static sp<Mat4Impl> ensureImpl(const sp<Mat4>& self);

};

}

#endif
