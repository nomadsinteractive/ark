#include "core/util/input_type.h"

#include "core/inf/array.h"
#include "core/impl/input/input_variable_array.h"

#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class InputArray : public Input {
public:
    InputArray(std::vector<sp<Input>> inputs)
        : _inputs(std::move(inputs)), _stride(_inputs.empty() ? 0 : _inputs.front()->size()) {
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const sp<Input>& i : _inputs)
            dirty = i->update(timestamp) || dirty;
        return dirty;
    }

    virtual void flat(void* buf) override {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
        for(const sp<Input>& i : _inputs) {
            CHECK(i->size() == _stride, "Input array should have same stride(%ld), but this one doesn't(%ld)", _stride, i->size());
            i->flat(ptr);
            ptr += _stride;
        }
    }

    virtual uint32_t size() override {
        return static_cast<uint32_t>(_stride * _inputs.size());
    }

private:
    std::vector<sp<Input>> _inputs;
    uint32_t _stride;
};

}

sp<Input> InputType::create(sp<Input> value)
{
    return sp<InputWrapper>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<sp<Mat4>> value)
{
    return sp<InputVariableArray<M4>>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<sp<Vec4>> value)
{
    return sp<InputVariableArray<V4>>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<sp<Input>> value)
{
    return sp<InputArray>::make(std::move(value));
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
    DCHECK(impl, "This Animate object is not a AnimateWrapper instance");
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
