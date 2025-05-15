#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Uploader")]]
class ARK_API UploaderType {
public:
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<ByteArray> value = nullptr);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<FloatArray> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<IntArray> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Integer> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Numeric> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Vec2> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Vec3> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Vec4> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(Vector<sp<Mat4>> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(Vector<sp<Uploader>> value);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(Vector<uint32_t> value);

//  [[script::bindings::classmethod]]
    static Vector<uint8_t> toBytes(Uploader& self);
    static Map<size_t, Vector<uint8_t>> record(Uploader& self);
    static Map<size_t, size_t> recordRanges(Uploader& self);
    static void writeTo(Uploader& self, void* ptr);

//  [[script::bindings::property]]
    static size_t size(const sp<Uploader>& self);

//  [[script::bindings::classmethod]]
    static sp<Uploader> reserve(sp<Uploader> self, size_t size);
//  [[script::bindings::classmethod]]
    static sp<Uploader> repeat(sp<Uploader> self, size_t length, size_t stride = 0);

//  [[script::bindings::map(set)]]
    static void put(const sp<Uploader>& self, size_t offset, sp<Uploader> uploader);
//  [[script::bindings::map(del)]]
    static void remove(const sp<Uploader>& self, size_t offset);
//  [[script::bindings::classmethod]]
    static void markDirty(const sp<Uploader>& self);

//  [[script::bindings::classmethod]]
    static void reset(const sp<Uploader>& self, sp<Uploader> delegate);
//  [[script::bindings::classmethod]]
    static sp<Uploader> wrap(sp<Uploader> self);

//  [[script::bindings::classmethod]]
    static sp<Uploader> dye(sp<Uploader> self, String message = "");

//  [[script::bindings::auto]]
    static sp<Uploader> makeElementIndexInput(Vector<element_index_t> value);

private:
    static sp<UploaderWrapper> ensureWrapper(const sp<Uploader>& self);
};

}
