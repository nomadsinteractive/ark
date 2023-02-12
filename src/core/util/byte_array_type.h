#ifndef ARK_CORE_UTIL_ASSET_TYPE_H_
#define ARK_CORE_UTIL_ASSET_TYPE_H_

#include "core/forwarding.h"
#include "core/base/slice.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("ByteArray")]]
class ARK_API ByteArrayType final {
public:

//  [[script::bindings::property]]
    static size_t nativePtr(const sp<ByteArray>& self);

//  [[script::bindings::map(len)]]
    static size_t len(const sp<ByteArray>& self);

//  [[script::bindings::map([])]]
    static uint8_t subscribe(const sp<ByteArray>& self, ptrdiff_t index);
//  [[script::bindings::map([])]]
    static sp<ByteArray> subscribe(sp<ByteArray> self, const Slice& slice);

//  [[script::bindings::classmethod]]
    static Span toBytes(const sp<ByteArray>& self);

//  [[script::bindings::classmethod]]
    static sp<Integer> toInteger(sp<ByteArray> self);
//  [[script::bindings::classmethod]]
    static sp<Numeric> toNumeric(sp<ByteArray> self);

//  [[script::bindings::classmethod]]
    static sp<Vec2> toVec2(sp<ByteArray> self);
//  [[script::bindings::classmethod]]
    static sp<Vec3> toVec3(sp<ByteArray> self);
//  [[script::bindings::classmethod]]
    static sp<Vec4> toVec4(sp<ByteArray> self);

};

}

#endif
