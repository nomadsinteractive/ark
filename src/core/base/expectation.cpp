#include "core/base/expectation.h"

namespace ark {

Expectation::Expectation(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier)
    : _delegate(delegate), _expectation(expectation), _notifier(std::move(notifier))
{
}

sp<Numeric> Expectation::toNumeric() const
{
    return _delegate;
}

const sp<Numeric>& Expectation::delegate() const
{
    return _delegate;
}

const sp<Numeric>& Expectation::expectation() const
{
    return _expectation;
}

const Notifier& Expectation::notifier() const
{
    return _notifier;
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
