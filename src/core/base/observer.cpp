#include "core/base/observer.h"

#include "core/inf/runnable.h"

namespace ark {

Observer::Observer(const sp<Runnable>& handler, bool oneshot)
    : _handler(handler), _oneshot(oneshot), _updated(true)
{
}

bool Observer::val()
{
    return dirty();
}

bool Observer::dirty()
{
    bool notified = _updated;
    _updated = false;
    return notified;
}

void Observer::update()
{
    _updated = true;
    sp<Runnable> handler = std::move(_handler);
    if(handler)
        handler->run();
    if(!_oneshot)
        _handler = std::move(handler);
}

}
