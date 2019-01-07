#include "core/base/observer.h"

#include "core/inf/runnable.h"

namespace ark {

Observer::Observer(bool dirty, const sp<Runnable>& handler, bool oneshot)
    : _handler(handler), _oneshot(oneshot), _dirty(dirty)
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
    sp<Runnable> handler = std::move(_handler);
    if(handler)
        handler->run();
    if(!_oneshot)
        _handler = std::move(handler);
}

}
