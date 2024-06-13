#include "core/traits/with_observer.h"

#include "core/base/observer.h"

namespace ark {

WithObserver::WithObserver(sp<Observer> observer)
    : _observer(std::move(observer))
{
}

const sp<Observer>& WithObserver::observer()
{
    if(!_observer)
        _observer = sp<Observer>::make();
    return _observer;
}

void WithObserver::notify() const
{
    if(_observer)
        _observer->notify();
}

}
