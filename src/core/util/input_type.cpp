#include "core/util/input_type.h"


#include "graphics/forwarding.h"

namespace ark {

sp<Input> InputType::create(const sp<Input>& value)
{
    return sp<InputWrapper>::make(value);
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
