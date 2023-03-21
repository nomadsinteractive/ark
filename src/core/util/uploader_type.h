#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/uploader.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Uploader")]]
class ARK_API InputType {
public:
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<ByteArray> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<IntArray> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Integer> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Numeric> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Vec2> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Vec3> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(sp<Vec4> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(std::map<size_t, sp<Uploader>> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(std::vector<sp<Mat4>> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(std::vector<sp<Uploader>> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(std::vector<V3> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(std::vector<V4> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(std::vector<uint32_t> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Uploader> create(const std::set<uint32_t>& value, size_t size = 0);

//  [[script::bindings::property]]
    static size_t size(const sp<Uploader>& self);

//  [[script::bindings::classmethod]]
    static sp<Uploader> reserve(sp<Uploader> self, size_t size);
//  [[script::bindings::classmethod]]
    static sp<Uploader> remap(sp<Uploader> self, size_t size, size_t offset = 0);
//  [[script::bindings::classmethod]]
    static sp<Uploader> repeat(sp<Uploader> self, size_t length, size_t stride = 0);

//  [[script::bindings::classmethod]]
    static void addInput(const sp<Uploader>& self, size_t offset, sp<Uploader> input);
//  [[script::bindings::classmethod]]
    static void removeInput(const sp<Uploader>& self, size_t offset);
//  [[script::bindings::classmethod]]
    static void markDirty(const sp<Uploader>& self);

//  [[script::bindings::classmethod]]
    static void reset(const sp<Uploader>& self, sp<Uploader> delegate);
//  [[script::bindings::classmethod]]
    static sp<Uploader> wrap(sp<Uploader> self);

//  [[script::bindings::auto]]
    static sp<Uploader> makeElementIndexInput(std::vector<element_index_t> value);
//  [[script::bindings::auto]]
    static sp<Uploader> blank(size_t size, int32_t fill = 0);

private:
    static sp<UploaderWrapper> ensureWrapper(const sp<Uploader>& self);
};

}
