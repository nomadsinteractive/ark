#include "core/base/observer.h"

#include "core/inf/runnable.h"

namespace ark {

Observer::Observer(const sp<Runnable>& callback, bool oneshot)
    : Observer(true, callback, oneshot)
{
}

Observer::Observer(bool dirty, const sp<Runnable>& callback, bool oneshot)
    : _callback(callback), _oneshot(oneshot), _dirty(dirty)
{
}

bool Observer::val()
{
    return dirty();
}

bool Observer::dirty()
{
    bool notified = _dirty;
    _dirty = false;
    return notified;
}

void Observer::update()
{
    _dirty = true;
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
