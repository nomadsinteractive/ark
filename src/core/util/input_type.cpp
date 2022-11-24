#include "core/util/input_type.h"

#include "core/inf/array.h"
#include "core/impl/input/input_object_array.h"
#include "core/impl/input/input_variable_array.h"
#include "core/impl/input/input_variable.h"

#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class InputArray : public Input {
public:
    InputArray(std::vector<sp<Input>> inputs)
        : _size(0) {
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

    virtual void flat(void* buf) override {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
        for(const auto& i : _inputs)
            i.second->flat(ptr + i.first);
    }

    virtual uint32_t size() override {
        return static_cast<uint32_t>(_size);
    }

private:
    std::vector<std::pair<size_t, sp<Input>>> _inputs;
    size_t _size;
};

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

void InputType::set(const sp<Input>& self, const sp<Input>& delegate)
{
    ensureImpl(self)->setDelegate(delegate);
}

uint32_t InputType::size(const sp<Input>& self)
{
    return self->size();
}

sp<InputType::InputWrapper> InputType::ensureImpl(const sp<Input>& self)
{
    const sp<InputType::InputWrapper> impl = self.as<InputType::InputWrapper>();
    DCHECK(impl, "This Input object is not a InputWrapper instance");
    return impl;
}

InputType::InputWrapper::InputWrapper(sp<Input> delegate)
    : _delegate(std::move(delegate)) {
}

bool InputType::InputWrapper::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void InputType::InputWrapper::flat(void* buf)
{
    _delegate->flat(buf);
}

uint32_t InputType::InputWrapper::size()
{
    return _delegate->size();
}

void InputType::InputWrapper::setDelegate(sp<Input> delegate)
{
    _delegate = std::move(delegate);
}

}
