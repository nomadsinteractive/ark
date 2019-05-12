#include "core/base/expectation.h"

namespace ark {

Expectation::Expectation(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier)
    : _delegate(delegate), _expectation(expectation), _notifier(std::move(notifier))
{
}

float Expectation::val()
{
    return _delegate->val();
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

const sp<Observer>& Expectation::addObserver(const sp<Runnable>& callback, bool oneshot)
{
    _observers.push_back(_notifier.createObserver(false, callback, oneshot));
    return _observers.back();
}

}
