#include "graphics/util/animate_type.h"


#include "graphics/forwarding.h"

namespace ark {

sp<Animate> AnimateType::create(const sp<Animate>& value)
{
    return sp<AnimateWrapper>::make(value);
}

sp<AnimateType::AnimateWrapper> AnimateType::ensureImpl(const sp<Animate>& self)
{
    const sp<AnimateType::AnimateWrapper> impl = self.as<AnimateType::AnimateWrapper>();
    DCHECK(impl, "This Animate object is not a AnimateWrapper instance");
    return impl;
}

AnimateType::AnimateWrapper::AnimateWrapper(sp<Animate> delegate)
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

void AnimateType::AnimateWrapper::setDelegate(sp<Animate> delegate)
{
    _delegate = std::move(delegate);
}

}
