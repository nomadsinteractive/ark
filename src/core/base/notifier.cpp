#include "core/base/notifier.h"

#include "core/base/with_timestamp.h"
#include "core/base/observer.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"

namespace ark {

namespace {

class Signal : public Boolean, public Runnable {
public:
    Signal(bool value)
        : _value(value) {
    }

    void run() override {
        _value.reset(!_value.value());
    }

    bool val() override {
        return _value.value();
    }

    bool update(uint64_t timestamp) override {
        return _value.update(timestamp);
    }

private:
    WithTimestamp<bool> _value;
};

}


Notifier::Notifier()
    : _observers(sp<List<sp<Observer>, ObserverFilter>>::make())
{
}

void Notifier::notify() const
{
    const auto observers = _observers;
    for(const sp<Observer>& i : *observers)
        i->notify();
}

sp<Observer> Notifier::makeObserver(const sp<Runnable>& handler, bool oneshot) const
{
    sp<Observer> observer = sp<Observer>::make(handler, oneshot);
    _observers->emplace_back(observer, nullptr);
    return observer;
}

sp<Boolean> Notifier::makeSignal(bool dirty) const
{
    sp<Signal> dirtyFlag = sp<Signal>::make(dirty);
    _observers->emplace_back(sp<Observer>::make(dirtyFlag, false), dirtyFlag);
    return dirtyFlag;
}

Notifier::ObserverFilter::ObserverFilter(sp<Boolean> dirtyFlag)
    : _dirty_flag(std::move(dirtyFlag))
{
}

FilterAction Notifier::ObserverFilter::operator() (const sp<Observer>& observer) const
{
    if(_dirty_flag)
        return _dirty_flag.unique() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
    return observer.unique() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
}

}
