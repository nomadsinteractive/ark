#include "core/base/notifier.h"

#include "core/inf/runnable.h"

namespace ark {

Notifier::Notifier(const sp<Runnable>& notifier)
    : _runnable(notifier), _notifying(false)
{
}

void Notifier::notify()
{
    if(_runnable && !_notifying)
    {
        _notifying = true;
        _runnable->run();
        _notifying = false;
    }
}

void Notifier::notifyOnce()
{
    const sp<Runnable> runnable = std::move(_runnable);
    if(runnable)
        runnable->run();
}

}
