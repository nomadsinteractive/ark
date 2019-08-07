#include "core/base/observer.h"

#include "core/inf/runnable.h"
#include "core/util/holder_util.h"

namespace ark {

Observer::Observer(const sp<Runnable>& callback, bool oneshot)
    : _callback(callback), _oneshot(oneshot)
{
}

int32_t Observer::traverse(const Holder::Visitor& visitor)
{
    return HolderUtil::visit(_callback, visitor);
}

int32_t Observer::clear()
{
    _callback = nullptr;
    return 0;
}

void Observer::update()
{
    sp<Runnable> callback = std::move(_callback);
    if(callback)
        callback->run();
    if(!_oneshot)
        _callback = std::move(callback);
}

void Observer::setCallback(const sp<Runnable>& callback)
{
    _callback = callback;
}

}
