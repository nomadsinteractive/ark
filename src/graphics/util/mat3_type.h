#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

namespace ark {

//[[script::bindings::class("Mat3")]]
class ARK_API Mat3Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Mat3> create(V3 t, V3 b, V3 n);
//  [[script::bindings::constructor]]
    static sp<Mat3> create(sp<Vec3> t = nullptr, sp<Vec3> b = nullptr, sp<Vec3> n = nullptr);

//  [[script::bindings::property]]
    static M3 val(const sp<Mat3>& self);

//  [[script::bindings::operator(@)]]
    static sp<Mat3> matmul(sp<Mat3> lvalue, sp<Mat3> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(sp<Mat3> lvalue, sp<Vec3> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec3> matmul(sp<Mat3> lvalue, V3 rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec2> matmul(sp<Mat3> lvalue, sp<Vec2> rvalue);
//  [[script::bindings::operator(@)]]
    static sp<Vec2> matmul(sp<Mat3> lvalue, V2 rvalue);

//  [[script::bindings::auto]]
    static sp<Mat3> identity();

//  [[script::bindings::classmethod]]
    static M3 update(const sp<Mat3>& self);
//  [[script::bindings::classmethod]]
    static sp<Mat3> freeze(const sp<Mat3>& self);

private:
    static sp<Mat3> ensureImpl(const sp<Mat3>& self);

};

}
