#include "core/base/observer.h"

#include "core/base/with_timestamp.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"

namespace ark {

namespace {

class Signal final : public Boolean, public Runnable {
public:
    Signal(const bool value, const bool signalValue)
        : _value(value), _signal_value(signalValue) {
    }

    void run() override {
        _value.reset(_signal_value);
    }

    bool val() override {
        return _value.value();
    }

    bool update(uint64_t timestamp) override {
        return _value.update(timestamp);
    }

private:
    WithTimestamp<bool> _value;
    bool _signal_value;
};

}

void Observer::run()
{
    notify();
}

void Observer::notify()
{
    Vector<Callback> callbacks = std::move(_callbacks);
    for(auto& [func, oneshot, owned, triggerAfter] : callbacks)
    {
        if(triggerAfter < 2)
            func->run();
        if(triggerAfter > 1 || !(oneshot || (owned && func.unique())))
            _callbacks.push_back({std::move(func), oneshot, owned, triggerAfter ? triggerAfter - 1 : 0});
    }
}

void Observer::addCallback(sp<Runnable> callback, const bool oneshot, const uint32_t countDown)
{
    ASSERT(callback);
    _callbacks.push_back({std::move(callback), oneshot, false, countDown});
}

sp<Boolean> Observer::addBooleanSignal(bool value, const bool oneshot)
{
    sp<Signal> signal = sp<Signal>::make(value, !value);
    _callbacks.push_back({signal, oneshot, true, 1});
    return signal;
}

}
