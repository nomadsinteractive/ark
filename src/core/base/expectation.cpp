#include "core/base/expectation.h"

namespace ark {

Expectation::Expectation(const sp<Numeric>& delegate, Notifier notifier)
    : _delegate(delegate), _notifier(std::move(notifier))
{
}

float Expectation::val()
{
    return _delegate->val();
}

const sp<Numeric>& Expectation::delegate() const
{
    return _delegate;
}

void Expectation::update()
{
    _notifier.notify();
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
