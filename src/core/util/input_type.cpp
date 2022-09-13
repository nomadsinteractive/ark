#include "core/util/input_type.h"

#include "core/inf/array.h"
#include "core/impl/input/input_variable_array.h"

#include "graphics/base/v4.h"
#include "graphics/impl/input/input_mat4fv.h"
#include "graphics/impl/input/input_v4f.h"

namespace ark {

sp<Input> InputType::create(sp<Input> value)
{
    return sp<InputWrapper>::make(std::move(value));
}

sp<Input> InputType::create(std::vector<sp<Mat4>> value)
{
    return sp<InputMat4fv>::make(sp<Array<sp<Mat4>>::Vector>::make(std::move(value)));
}

sp<Input> InputType::create(std::vector<sp<Vec4>> value)
{
    return sp<InputVariableArray<V4>>::make(std::move(value));
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
