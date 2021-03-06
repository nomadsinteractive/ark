#ifndef ARK_GRAPHICS_UTIL_MAT4_TYPE_H_
#define ARK_GRAPHICS_UTIL_MAT4_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//  [[script::bindings::class("Mat4")]]
class ARK_API Mat4Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Mat4> create(const V4& t, const V4& b, const V4& n, const V4& w);
//  [[script::bindings::constructor]]
    static sp<Mat4> create(const sp<Vec4>& t = nullptr, const sp<Vec4>& b = nullptr, const sp<Vec4>& n = nullptr, const sp<Vec4>& w = nullptr);

//  [[script::bindings::operator(*)]]
    static sp<Mat4> mul(const sp<Mat4>& lvalue, const sp<Mat4>& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Mat4> matmul(const sp<Mat4>& lvalue, const sp<Mat4>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(const sp<Mat4>& lvalue, const sp<Vec4>& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec4> matmul(const sp<Mat4>& lvalue, const sp<Vec4>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec4> mul(const sp<Mat4>& lvalue, const V4& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec4> matmul(const sp<Mat4>& lvalue, const V4& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Mat4>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(const sp<Mat4>& lvalue, const sp<Vec3>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec3> mul(const sp<Mat4>& lvalue, const V3& rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(const sp<Mat4>& lvalue, const V3& rvalue);

//  [[script::bindings::auto]]
    static sp<Mat4> identity();

//  [[script::bindings::classmethod]]
    static void fix(const sp<Mat4>& self);

private:
    static sp<Mat4Impl> ensureImpl(const sp<Mat4>& self);

};

}

#endif
