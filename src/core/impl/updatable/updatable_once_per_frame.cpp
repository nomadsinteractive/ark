#include "core/impl/updatable/updatable_once_per_frame.h"

namespace ark {

UpdatableOncePerFrame::UpdatableOncePerFrame(sp<Updatable> delegate)
    : _delegate(std::move(delegate)), _last_update_value(false), _last_update_timestamp(0)
{
}

bool UpdatableOncePerFrame::update(const uint64_t timestamp)
{
    if(timestamp <= _last_update_timestamp)
        return _last_update_value;

    _last_update_value = _delegate->update(timestamp);
    _last_update_timestamp = timestamp;
    return _last_update_value;
}

}
