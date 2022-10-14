#include "core/impl/updatable/updatable_wrapper.h"

namespace ark {

UpdatableWrapper::UpdatableWrapper(sp<Updatable> updatable)
    : Delegate<Updatable>(std::move(updatable))
{
}

bool UpdatableWrapper::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

}
