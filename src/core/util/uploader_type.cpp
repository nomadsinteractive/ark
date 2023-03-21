#include "core/util/uploader_type.h"

#include "core/inf/array.h"
#include "core/impl/uploader/input_impl.h"
#include "core/impl/uploader/uploader_array.h"
#include "core/impl/uploader/input_repeat.h"
#include "core/impl/uploader/input_variable_array.h"
#include "core/impl/uploader/input_variable.h"
#include "core/impl/writable/writable_memory.h"
#include "core/impl/writable/writable_with_offset.h"
#include "core/impl/uploader/uploader_wrapper.h"

#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class InputArray : public Uploader {
public:
    InputArray(std::vector<sp<Uploader>> inputs)
        : Uploader(0) {
        for(sp<Uploader>& i : inputs) {
            size_t size = i->size();
            _inputs.emplace_back(_size, std::move(i));
            _size += size;
        }
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const auto& i : _inputs)
            dirty = i.second->update(timestamp) || dirty;
        return dirty;
    }

    virtual void upload(Writable& buf) override {
        for(const auto& i : _inputs) {
            WritableWithOffset wwo(buf, i.first);
            i.second->upload(wwo);
        }
    }

private:
    std::vector<std::pair<size_t, sp<Uploader>>> _inputs;
};

}

static sp<InputImpl> ensureImpl(const sp<Uploader>& self)
{
    const sp<InputImpl> impl = self.as<InputImpl>();
    CHECK(impl, "This object is not a InputImpl instance. Use \"reserve\" method to create an InputImpl instance.");
    return impl;
}


sp<Uploader> InputType::create(sp<ByteArray> value, size_t size)
{
    return reserve(sp<UploaderArray<uint8_t>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(sp<IntArray> value, size_t size)
{
    return reserve(sp<UploaderArray<int32_t>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(sp<Integer> value, size_t size)
{
    return reserve(sp<InputVariable<int32_t>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(sp<Numeric> value, size_t size)
{
    return reserve(sp<InputVariable<float>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(sp<Vec2> value, size_t size)
{
    return reserve(sp<InputVariable<V2>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(sp<Vec3> value, size_t size)
{
    return reserve(sp<InputVariable<V3>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(sp<Vec4> value, size_t size)
{
    return reserve(sp<InputVariable<V4>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(std::map<size_t, sp<Uploader>> value, size_t size)
{
    return sp<InputImpl>::make(std::move(value), size);
}

sp<Uploader> InputType::create(std::vector<sp<Mat4>> value, size_t size)
{
    return reserve(sp<InputVariableArray<M4>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(std::vector<sp<Uploader>> value, size_t size)
{
    return reserve(sp<InputArray>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(std::vector<V3> value, size_t size)
{
    return reserve(sp<UploaderArray<V3>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(std::vector<V4> value, size_t size)
{
    return reserve(sp<UploaderArray<V4>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(std::vector<uint32_t> value, size_t size)
{
    return reserve(sp<UploaderArray<uint32_t>>::make(std::move(value)), size);
}

sp<Uploader> InputType::create(const std::set<uint32_t>& value, size_t size)
{
    return reserve(sp<UploaderArray<uint32_t>>::make(std::vector<uint32_t>(value.begin(), value.end())), size);
}

sp<Uploader> InputType::wrap(sp<Uploader> self)
{
    return sp<UploaderWrapper>::make(std::move(self));
}

sp<Uploader> InputType::makeElementIndexInput(std::vector<element_index_t> value)
{
    return sp<UploaderArray<element_index_t>>::make(std::move(value));
}

sp<Uploader> InputType::blank(size_t size, int32_t fill)
{
    return sp<UploaderArray<int8_t>>::make(std::vector<int8_t>(size, static_cast<int8_t>(fill)));
}

void InputType::reset(const sp<Uploader>& self, sp<Uploader> delegate)
{
    ensureWrapper(self)->setDelegate(std::move(delegate));
}

size_t InputType::size(const sp<Uploader>& self)
{
    return self->size();
}

sp<Uploader> InputType::reserve(sp<Uploader> self, size_t size)
{
    if(size == 0)
        return self;

    ASSERT(size >= self->size());
    sp<InputImpl> inputImpl = sp<InputImpl>::make(size);
    if(self->size() > 0)
        inputImpl->addInput(0,  std::move(self));
    return inputImpl;
}

sp<Uploader> InputType::remap(sp<Uploader> self, size_t size, size_t offset)
{
    sp<InputImpl> inputImpl = sp<InputImpl>::make(size);
    if(self->size() > 0)
        inputImpl->addInput(offset,  std::move(self));
    return inputImpl;
}

sp<Uploader> InputType::repeat(sp<Uploader> self, size_t length, size_t stride)
{
    return sp<InputRepeat>::make(std::move(self), length, stride);
}

void InputType::addInput(const sp<Uploader>& self, size_t offset, sp<Uploader> input)
{
    ensureImpl(self)->addInput(offset, std::move(input));
}

void InputType::removeInput(const sp<Uploader>& self, size_t offset)
{
    ensureImpl(self)->removeInput(offset);
}

void InputType::markDirty(const sp<Uploader>& self)
{
    ensureImpl(self)->markDirty();
}

sp<UploaderWrapper> InputType::ensureWrapper(const sp<Uploader>& self)
{
    const sp<UploaderWrapper> wrapper = self.as<UploaderWrapper>();
    CHECK(wrapper, "This Input object is not a InputWrapper instance");
    return wrapper;
}

}
