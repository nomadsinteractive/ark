#ifndef ARK_GRAPHICS_UTIL_VV2_UTIL_H_
#define ARK_GRAPHICS_UTIL_VV2_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

namespace ark {

//[[script::bindings::class("VV2")]]
class ARK_API VV2Util final {
public:
//[[script::bindings::constructor]]
    static sp<VV2> create(const sp<Numeric>& x, const sp<Numeric>& y);
//[[script::bindings::constructor]]
    static sp<VV2> create(float x, float y);

//[[script::bindings::operator(+)]]
    static sp<VV2> add(const sp<VV2>& self, const sp<VV2>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<VV2> sub(const sp<VV2>& self, const sp<VV2>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<VV2> mul(const sp<VV2>& self, const sp<VV2>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<VV2> truediv(const sp<VV2>& self, const sp<VV2>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<VV2> floordiv(const sp<VV2>& self, const sp<VV2>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<VV2> negative(const sp<VV2>& self);

//[[script::bindings::classmethod]]
    static sp<VV2> transform(const sp<VV2>& self, const sp<Transform>& transform, const sp<VV2>& org);

//[[script::bindings::property]]
    static V2 val(const sp<VV2>& self);

};

}

#endif
