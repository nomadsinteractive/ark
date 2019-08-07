#include "core/base/expectation.h"

#include "core/base/observer.h"

namespace ark {

Expectation::Expectation(const sp<Numeric>& delegate, Notifier notifier)
    : Delegate<Numeric>(delegate), _notifier(std::move(notifier))
{
}

float Expectation::val()
{
    return _delegate->val();
}

int32_t Expectation::traverse(const Holder::Visitor& visitor)
{
    for(const sp<Observer>& i : _observers)
    {
        int32_t r = i->traverse(visitor);
        if(r)
            return r;
    }
    return 0;
}

int32_t Expectation::clear()
{
    for(const sp<Observer>& i : _observers)
        i->clear();
    return 0;
}

sp<Observer> Expectation::createObserver(const sp<Runnable>& callback, bool oneshot)
{
    return _notifier.createObserver(callback, oneshot);
}

const sp<Observer>& Expectation::addObserver(const sp<Runnable>& callback, bool oneshot)
{
    _observers.push_back(_notifier.createObserver(callback, oneshot));
    return _observers.back();
}

}
