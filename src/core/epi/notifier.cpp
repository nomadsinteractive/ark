#include "core/epi/notifier.h"

#include "core/base/observer.h"

namespace ark {

Notifier::Notifier()
    : _observers(sp<WeakRefList<Observer>>::make())
{
}

void Notifier::notify()
{
    for(const sp<Observer>& i : *_observers)
        i->update();
}

sp<Observer> Notifier::createObserver(bool dirty, const sp<Runnable>& handler, bool oneshot)
{
    const sp<Observer> observer = sp<Observer>::make(dirty, handler, oneshot);
    _observers->push_back(observer);
    return observer;
}

}
