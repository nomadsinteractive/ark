#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

namespace ark {

//[[script::bindings::class("Mat2")]]
class ARK_API Mat2Type final {
public:
//  [[script::bindings::constructor]]
    static sp<Mat2> create(V2 t, V2 b);
//  [[script::bindings::constructor]]
    static sp<Mat2> create(const sp<Vec2>& t = nullptr, const sp<Vec2>& b = nullptr);

//  [[script::bindings::property]]
    static M2 val(const sp<Mat2>& self);

//  [[script::bindings::operator(*)]]
    static sp<Mat2> mul(const sp<Mat2>& lvalue, const sp<Mat2>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Mat2>& lvalue, const sp<Vec2>& rvalue);
//  [[script::bindings::operator(*)]]
    static sp<Vec2> mul(const sp<Mat2>& lvalue, const V2& rvalue);

//  [[script::bindings::classmethod]]
    static sp<Mat2> rotate(const sp<Mat2>& self, const sp<Numeric>& radians);

//  [[script::bindings::classmethod]]
    static sp<Mat2> ifElse(const sp<Mat2>& self, const sp<Boolean>& condition, const sp<Mat2>& negative);

//  [[script::bindings::classmethod]]
    static M2 update(const sp<Mat2>& self);
//  [[script::bindings::classmethod]]
    static sp<Mat2> freeze(const sp<Mat2>& self);
};

}
