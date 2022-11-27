#include "core/util/input_type.h"

#include "core/inf/array.h"
#include "core/impl/input/input_impl.h"
#include "core/impl/input/input_object_array.h"
#include "core/impl/input/input_repeat.h"
#include "core/impl/input/input_variable_array.h"
#include "core/impl/input/input_variable.h"
#include "core/impl/writable/writable_memory.h"
#include "core/impl/writable/writable_with_offset.h"

#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class InputArray : public Input {
public:
    InputArray(std::vector<sp<Input>> inputs)
        : Input(0) {
        for(sp<Input>& i : inputs) {
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
        for(const auto& i : _inputs)
            i.second->upload(WritableWithOffset(buf, i.first));
    }

private:
    std::vector<std::pair<size_t, sp<Input>>> _inputs;
};

}

static sp<InputImpl> ensureImpl(const sp<Input>& self)
{
    const sp<InputImpl> impl = self.as<InputImpl>();
    CHECK(impl, "This object is not a InputImpl instance. Use \"shift\" method to create an InputImpl instance.");
    return impl;
}


sp<Input> InputType::create(sp<ByteArray> value)
{
    return sp<InputObjectArray<uint8_t>>::make(std::move(value));
}

sp<Input> InputType::create(sp<Integer> value)
{
    return sp<InputVariable<int32_t>>::make(std::move(value));
}

sp<Input> InputType::create(sp<Numeric> value)
{
    return sp<InputVariable<float>>::make(std::move(value));
}

sp<Input> InputType::create(sp<Vec2> value)
{
    return sp<InputVariable<V2>>::make(std::move(value));
}

sp<Input> InputType::create(sp<Vec3> value)
{
    return sp<InputVariable<V3>>::make(std::move(value));
}

sp<Input> InputType::create(sp<Vec4> value)
{
    return sp<InputVariable<V4>>::make(std::move(value));
}

sp<Input> InputType::create(std::map<size_t, sp<Input>> value)
{
    return sp<InputImpl>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<sp<Mat4>> value)
{
    return sp<InputVariableArray<M4>>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<sp<Input>> value)
{
    return sp<InputArray>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<V3> value)
{
    return sp<InputObjectArray<V3>>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<V4> value)
{
    return sp<InputObjectArray<V4>>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<uint32_t> value)
{
    return sp<InputObjectArray<uint32_t>>::make(std::move(value));
}

sp<Input> InputType::wrap(sp<Input> self)
{
    return sp<InputWrapper>::make(std::move(self));
}

sp<Input> InputType::makeElementIndexInput(std::vector<element_index_t> value)
{
    return sp<InputObjectArray<element_index_t>>::make(std::move(value));
}

void InputType::set(const sp<Input>& self, sp<Input> delegate)
{
    ensureWrapper(self)->setDelegate(std::move(delegate));
}

size_t InputType::size(const sp<Input>& self)
{
    return self->size();
}

sp<Input> InputType::shift(const sp<Input>& self, size_t offset, size_t size)
{
    sp<InputImpl> inputImpl = sp<InputImpl>::make(size ? size : offset + self->size());
    if(self->size() > 0)
        inputImpl->addInput(offset,  self);
    return inputImpl;
}

sp<Input> InputType::repeat(sp<Input> self, size_t length, size_t stride)
{
    return sp<InputRepeat>::make(std::move(self), length, stride);
}

void InputType::addInput(const sp<Input>& self, size_t offset, sp<Input> input)
{
    ensureImpl(self)->addInput(offset, std::move(input));
}

void InputType::removeInput(const sp<Input>& self, size_t offset)
{
    ensureImpl(self)->removeInput(offset);
}

sp<InputType::InputWrapper> InputType::ensureWrapper(const sp<Input>& self)
{
    const sp<InputType::InputWrapper> impl = self.as<InputType::InputWrapper>();
    DCHECK(impl, "This Input object is not a InputWrapper instance");
    return impl;
}

InputType::InputWrapper::InputWrapper(sp<Input> delegate)
    : Input(delegate->size()), _delegate(std::move(delegate)) {
}

bool InputType::InputWrapper::update(uint64_t timestamp)
{
    return _delegate->update(timestamp) || _timestamp.update(timestamp);
}

void InputType::InputWrapper::upload(Writable& buf)
{
    _delegate->upload(buf);
}

void InputType::InputWrapper::setDelegate(sp<Input> delegate)
{
    _size = delegate->size();
    _delegate = std::move(delegate);
    _timestamp.setDirty();
}

}
