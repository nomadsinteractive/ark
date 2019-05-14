#include "core/base/observer.h"

#include "core/inf/runnable.h"

namespace ark {

Observer::Observer(const sp<Runnable>& callback, bool oneshot)
    : _callback(callback), _oneshot(oneshot)
{
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
