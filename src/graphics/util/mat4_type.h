#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

namespace ark {

//[[script::bindings::class("Mat4")]]
class ARK_API Mat4Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Mat4> create(const M4& other = M4());
//  [[script::bindings::constructor]]
    static sp<Mat4> create(sp<Mat4> other);
//  [[script::bindings::constructor]]
    static sp<Mat4> create(sp<Mat3> other);
    static sp<Mat4> create(V4 t, V4 b, V4 n, V4 w);
    static sp<Mat4> create(sp<Vec4> t, sp<Vec4> b, sp<Vec4> n, sp<Vec4> w);

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
    static sp<Vec4> matmul(sp<Mat4> lvalue, V4 rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(sp<Mat4> lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(sp<Mat4> lvalue, V3 rvalue);

//  [[script::bindings::classmethod]]
    static sp<Mat4> rotate(sp<Mat4> self, sp<Vec4> quaternion);
//  [[script::bindings::classmethod]]
    static sp<Mat4> scale(sp<Mat4> self, sp<Vec3> scale);
//  [[script::bindings::classmethod]]
    static sp<Mat4> translate(sp<Mat4> self, sp<Vec3> translation);

//  [[script::bindings::classmethod]]
    static sp<Mat4> inverse(sp<Mat4> self);

//  [[script::bindings::classmethod]]
    static M4 update(const sp<Mat4>& self);
//  [[script::bindings::classmethod]]
    static sp<Mat4> freeze(const sp<Mat4>& self);

//  [[script::bindings::classmethod]]
    static sp<Mat4> dye(sp<Mat4> self, sp<Boolean> condition = nullptr, String message = "");

private:
    static sp<Mat4Impl> ensureImpl(const sp<Mat4>& self);

};

}
