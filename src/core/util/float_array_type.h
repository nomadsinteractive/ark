#pragma once

#include "core/forwarding.h"
#include "core/base/slice.h"
#include "core/util/array_type.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("FloatArray")]]
class ARK_API FloatArrayType final : public ArrayType<float> {
public:

/*
//  [[script::bindings::constructor]]
    static sp<FloatArray> create(size_t length, float fill = 0);

//  [[script::bindings::classmethod]]
    static sp<FloatArray> wrap(sp<FloatArray> self);
//  [[script::bindings::classmethod]]
    static void reset(const sp<FloatArray>& self, sp<FloatArray> other);

//  [[script::bindings::property]]
    static size_t nativePtr(const sp<FloatArray>& self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<FloatArray>& self);
//  [[script::bindings::seq(get)]]
    static Optional<float> getItem(const sp<FloatArray>& self, ptrdiff_t index);
//  [[script::bindings::seq(set)]]
    static int32_t setItem(const sp<FloatArray>& self, ptrdiff_t index, float value);

//  [[script::bindings::map(get)]]
    static Optional<float> subscribe(const sp<FloatArray>& self, ptrdiff_t index);
//  [[script::bindings::map(get)]]
    static sp<FloatArray> subscribe(const sp<FloatArray>& self, const Slice& index);
//  [[script::bindings::map(set)]]
    static int32_t subscribeAssign(const sp<FloatArray>& self, ptrdiff_t index, float value);
//  [[script::bindings::map(set)]]
    static int32_t subscribeAssign(const sp<FloatArray>& self, const Slice& index, const std::vector<float>& value);

//  [[script::bindings::classmethod]]
    static Span toBytes(const sp<FloatArray>& self);
//  [[script::bindings::classmethod]]
    static sp<ByteArray> toByteArray(sp<FloatArray> self);
*/

};

}
