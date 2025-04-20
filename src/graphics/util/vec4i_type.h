#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Vec4i")]]
class ARK_API Vec4iType {
public:
//  [[script::bindings::constructor]]
    static sp<Vec4i> create(int32_t x, int32_t y, int32_t z, int32_t w);
//  [[script::bindings::constructor]]
    static sp<Vec4i> create(sp<Integer> x, sp<Integer> y = nullptr, sp<Integer> z = nullptr, sp<Integer> w = nullptr);

//  [[script::bindings::property]]
    static V4i val(const sp<Vec4i>& self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<Vec4i>& self);
//  [[script::bindings::seq(get)]]
    static Optional<int32_t> getItem(const sp<Vec4i>& self, ptrdiff_t index);

};

}
