#pragma once

#include "core/forwarding.h"
#include "core/util/array_type.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("ByteArray")]]
class ARK_API ByteArrayType final : public ArrayType<uint8_t> {
public:
/*
//  [[script::bindings::constructor]]
    static sp<ByteArray> create(size_t length, uint8_t fill = 0);

//  [[script::bindings::classmethod]]
    static sp<ByteArray> wrap(sp<ByteArray> self);
//  [[script::bindings::classmethod]]
    static void reset(const sp<ByteArray>& self, sp<ByteArray> other);

//  [[script::bindings::property]]
    static size_t nativePtr(const sp<ByteArray>& self);

//  [[script::bindings::seq(len)]]
    static size_t len(const sp<ByteArray>& self);
//  [[script::bindings::seq(get)]]
    static Optional<uint8_t> getItem(const sp<ByteArray>& self, ptrdiff_t index);
//  [[script::bindings::seq(set)]]
    static int32_t setItem(const sp<ByteArray>& self, ptrdiff_t index, uint8_t value);

//  [[script::bindings::map(get)]]
    static Optional<uint8_t> subscribe(const sp<ByteArray>& self, ptrdiff_t index);
//  [[script::bindings::map(get)]]
    static sp<ByteArray> subscribe(const sp<ByteArray>& self, const Slice& index);
//  [[script::bindings::map(set)]]
    static int32_t subscribeAssign(const sp<ByteArray>& self, ptrdiff_t index, uint8_t value);
//  [[script::bindings::map(set)]]
    static int32_t subscribeAssign(const sp<ByteArray>& self, const Slice& index, const Vector<uint8_t>& value);

//  [[script::bindings::classmethod]]
    static Span toBytes(const sp<ByteArray>& self);
*/

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

//  [[script::bindings::classmethod]]
    static sp<Mat2> toMat2(sp<ByteArray> self);
//  [[script::bindings::classmethod]]
    static sp<Mat3> toMat3(sp<ByteArray> self);
//  [[script::bindings::classmethod]]
    static sp<Mat4> toMat4(sp<ByteArray> self);
};

}
