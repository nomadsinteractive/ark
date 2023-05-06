#pragma once

#include "core/forwarding.h"
#include "core/base/slice.h"
#include "core/util/array_type.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("IntArray")]]
class ARK_API IntArrayType final : public ArrayType<int32_t> {
public:

/*
//  [[script::bindings::constructor]]
    static sp<IntArray> create(size_t length, int32_t fill = 0);

//  [[script::bindings::classmethod]]
    static sp<IntArray> wrap(sp<IntArray> self);
//  [[script::bindings::classmethod]]
    static void reset(const sp<IntArray>& self, sp<IntArray> other);

//  [[script::bindings::property]]
    static size_t nativePtr(const sp<IntArray>& self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<IntArray>& self);
//  [[script::bindings::seq(get)]]
    static Optional<int32_t> getItem(const sp<IntArray>& self, ptrdiff_t index);
//  [[script::bindings::seq(set)]]
    static int32_t setItem(const sp<IntArray>& self, ptrdiff_t index, int32_t value);

//  [[script::bindings::map(get)]]
    static Optional<int32_t> subscribe(const sp<IntArray>& self, ptrdiff_t index);
//  [[script::bindings::map(get)]]
    static sp<IntArray> subscribe(const sp<IntArray>& self, const Slice& slice);
//  [[script::bindings::map(set)]]
    static int32_t subscribeAssign(const sp<IntArray>& self, ptrdiff_t index, int32_t value);
//  [[script::bindings::map(set)]]
    static int32_t subscribeAssign(const sp<IntArray>& self, const Slice& slice, const std::vector<int32_t>& values);

//  [[script::bindings::classmethod]]
    static Span toBytes(const sp<IntArray>& self);
*/

};

}
