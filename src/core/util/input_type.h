#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/input.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//  [[script::bindings::class("Input")]]
class ARK_API InputType {
public:
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<ByteArray> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<IntArray> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<Integer> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<Numeric> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<Vec2> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<Vec3> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(sp<Vec4> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(std::map<size_t, sp<Input>> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(std::vector<sp<Mat4>> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(std::vector<sp<Input>> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(std::vector<V3> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(std::vector<V4> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(std::vector<uint32_t> value, size_t size = 0);
//  [[script::bindings::constructor]]
    static sp<Input> create(const std::set<uint32_t>& value, size_t size = 0);

//  [[script::bindings::property]]
    static size_t size(const sp<Input>& self);

//  [[script::bindings::classmethod]]
    static sp<Input> reserve(sp<Input> self, size_t size);
//  [[script::bindings::classmethod]]
    static sp<Input> remap(sp<Input> self, size_t size, size_t offset = 0);
//  [[script::bindings::classmethod]]
    static sp<Input> repeat(sp<Input> self, size_t length, size_t stride = 0);

//  [[script::bindings::classmethod]]
    static void addInput(const sp<Input>& self, size_t offset, sp<Input> input);
//  [[script::bindings::classmethod]]
    static void removeInput(const sp<Input>& self, size_t offset);
//  [[script::bindings::classmethod]]
    static void markDirty(const sp<Input>& self);

//  [[script::bindings::classmethod]]
    static void reset(const sp<Input>& self, sp<Input> delegate);
//  [[script::bindings::classmethod]]
    static sp<Input> wrap(sp<Input> self);

//  [[script::bindings::auto]]
    static sp<Input> makeElementIndexInput(std::vector<element_index_t> value);
//  [[script::bindings::auto]]
    static sp<Input> blank(size_t size, int32_t fill = 0);

private:
    class InputWrapper : public Input, Implements<InputWrapper, Input> {
    public:
        InputWrapper(sp<Input> delegate);

        virtual bool update(uint64_t timestamp) override;
        virtual void upload(Writable& buf) override;

        void setDelegate(sp<Input> delegate);

    private:
        sp<Input> _delegate;
        Timestamp _timestamp;
    };

private:
    static sp<InputWrapper> ensureWrapper(const sp<Input>& self);
};

}
