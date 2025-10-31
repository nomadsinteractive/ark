#include "core/base/observer.h"

#include "core/base/future.h"
#include "core/base/with_timestamp.h"
#include "core/inf/variable.h"

namespace ark {

namespace {

class Signal final : public Boolean, public Runnable {
public:
    Signal(const bool value)
        : _value(value) {
    }

    void run() override {
        _value.reset(!_value.val());
    }

    bool val() override {
        return _value.val();
    }

    bool update(uint32_t tick) override {
        return _value.update(tick);
    }

private:
    WithTimestamp<bool> _value;
};

}

void Observer::run()
{
    notify();
}

void Observer::notify()
{
    Vector<sp<Future>> futureCallbacks = std::move(_future_callbacks);
    for(sp<Future>& future : futureCallbacks)
    {
        future->notify();
        if(!future->isDoneOrCanceled()->val())
            _future_callbacks.push_back(std::move(future));
    }
}

void Observer::addCallback(sp<Runnable> callback)
{
    addFutureCallback(sp<Future>::make(std::move(callback), nullptr, 0));
}

void Observer::addFutureCallback(sp<Future> future)
{
    ASSERT(future);
    _future_callbacks.emplace_back(std::move(future));
}

sp<Boolean> Observer::addBooleanSignal(bool value)
{
    sp<Signal> signal = sp<Signal>::make(value);
    _future_callbacks.emplace_back(sp<Future>::make(signal));
    return signal;
}

}
