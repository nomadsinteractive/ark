#ifndef ARK_CORE_UTIL_INT_ARRAY_TYPE_H_
#define ARK_CORE_UTIL_INT_ARRAY_TYPE_H_

#include "core/forwarding.h"
#include "core/base/slice.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("IntArray")]]
class ARK_API IntArrayType final {
public:

//  [[script::bindings::constructor]]
    static sp<IntArray> create(size_t length, int32_t fill = 0);

//  [[script::bindings::classmethod]]
    static sp<IntArray> wrap(sp<IntArray> self);
//  [[script::bindings::classmethod]]
    static void reset(const sp<IntArray>& self, sp<IntArray> other);

//  [[script::bindings::property]]
    static size_t nativePtr(const sp<IntArray>& self);

//  [[script::bindings::map(len)]]
    static size_t len(const sp<IntArray>& self);

//  [[script::bindings::map(get)]]
    static int32_t subscribe(const sp<IntArray>& self, ptrdiff_t index);
//  [[script::bindings::map(get)]]
    static sp<IntArray> subscribe(sp<IntArray> self, const Slice& slice);

//  [[script::bindings::map(set)]]
    static int32_t setItem(sp<IntArray> self, ptrdiff_t index, int32_t value);
//  [[script::bindings::map(set)]]
    static int32_t setItem(sp<IntArray> self, const Slice& slice, const std::vector<int32_t>& values);

//  [[script::bindings::classmethod]]
    static Span toBytes(const sp<IntArray>& self);

private:
    static int32_t* getSubscriptionPtr(IntArray& self, ptrdiff_t index);

};

}

#endif
