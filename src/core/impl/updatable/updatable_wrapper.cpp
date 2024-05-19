#include "core/impl/updatable/updatable_wrapper.h"

namespace ark {

UpdatableWrapper::UpdatableWrapper(sp<Updatable> updatable)
    : Wrapper(std::move(updatable))
{
}

bool UpdatableWrapper::update(uint64_t timestamp)
{
    return (_wrapped && _wrapped->update(timestamp)) || _timestamp.update(timestamp);
}

void UpdatableWrapper::reset(sp<Updatable> delegate)
{
    _timestamp.markDirty();
    Wrapper::reset(std::move(delegate));
}

}
