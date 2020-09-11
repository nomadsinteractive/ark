#include "graphics/util/animate_type.h"


#include "graphics/forwarding.h"

namespace ark {

sp<Flatable> AnimateType::create(const sp<Flatable>& value)
{
    return sp<AnimateWrapper>::make(value);
}

void AnimateType::set(const sp<Flatable>& self, const sp<Flatable>& delegate)
{
    ensureImpl(self)->setDelegate(delegate);
}

uint32_t AnimateType::size(const sp<Flatable>& self)
{
    return self->size();
}

sp<AnimateType::AnimateWrapper> AnimateType::ensureImpl(const sp<Flatable>& self)
{
    const sp<AnimateType::AnimateWrapper> impl = self.as<AnimateType::AnimateWrapper>();
    DCHECK(impl, "This Animate object is not a AnimateWrapper instance");
    return impl;
}

AnimateType::AnimateWrapper::AnimateWrapper(sp<Flatable> delegate)
    : _delegate(std::move(delegate)) {
}

bool AnimateType::AnimateWrapper::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void AnimateType::AnimateWrapper::flat(void* buf)
{
    _delegate->flat(buf);
}

uint32_t AnimateType::AnimateWrapper::size()
{
    return _delegate->size();
}

void AnimateType::AnimateWrapper::setDelegate(sp<Flatable> delegate)
{
    _delegate = std::move(delegate);
}

}
