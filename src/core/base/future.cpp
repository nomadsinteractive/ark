#include "core/base/future.h"

#include "core/base/observer.h"

namespace ark {

Future::Future(sp<Boolean> canceled, sp<Observer> observer)
    : _canceled(canceled ? sp<VariableWrapper<bool>>::make(std::move(canceled)) : sp<VariableWrapper<bool>>::make(false)), _observer(std::move(observer)), _done(false) {
}

void Future::cancel()
{
    _canceled->set(true);
}

void Future::done()
{
    _done = true;
    if(_observer)
        _observer->notify();
}

bool Future::isCancelled() const
{
    return _canceled->val();
}

bool Future::isDone() const
{
    return _done;
}

sp<Boolean> Future::canceled() const
{
    return _canceled;
}

}
