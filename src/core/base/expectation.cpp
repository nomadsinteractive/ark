#include "core/base/expectation.h"

#include "core/base/observer.h"

namespace ark {

Expectation::Expectation(sp<Numeric> delegate, Notifier notifier)
    : Delegate<Numeric>(std::move(delegate)), _notifier(std::move(notifier))
{
}

float Expectation::val()
{
    return _delegate->val();
}

bool Expectation::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void Expectation::traverse(const Holder::Visitor& visitor)
{
    for(const sp<Observer>& i : _observers)
        i->traverse(visitor);
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

void Expectation::clear()
{
    _observers.clear();
}

}
