#include "core/impl/updatable/updatable_wrapper.h"

namespace ark {

UpdatableWrapper::UpdatableWrapper(sp<Updatable> updatable)
    : Wrapper<Updatable>(std::move(updatable))
{
}

bool UpdatableWrapper::update(uint64_t timestamp)
{
    return (_wrapped ? _wrapped->update(timestamp) : false) | _timestamp.update(timestamp);
}

void UpdatableWrapper::reset(sp<UpdatableWrapper> delegate)
{
    _timestamp.setDirty();
    Wrapper::reset(std::move(delegate));
}

}