#include "core/base/observer.h"

#include "core/inf/runnable.h"

namespace ark {

Observer::Observer(const sp<Runnable>& handler)
    : _handler(handler), _notifying(false)
{
}

void Observer::update()
{
    if(_handler && !_notifying)
    {
        _notifying = true;
        _handler->run();
        _notifying = false;
    }
}

void Observer::updateOnce()
{
    const sp<Runnable> runnable = std::move(_handler);
    if(runnable)
        runnable->run();
}

}
