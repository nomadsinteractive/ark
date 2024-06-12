#include "core/base/observer.h"

#include "core/inf/runnable.h"

namespace ark {

Observer::Observer(sp<Runnable> callback, bool oneshot)
    : _callback(std::move(callback)), _oneshot(oneshot)
{
}

void Observer::notify()
{
    sp<Runnable> callback = std::move(_callback);
    if(callback)
        callback->run();
    if(!_oneshot)
        _callback = std::move(callback);
}

const sp<Runnable>& Observer::callback()
{
    return _callback;
}

void Observer::setCallback(sp<Runnable> callback)
{
    _callback = std::move(callback);
}

}
