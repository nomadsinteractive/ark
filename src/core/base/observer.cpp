#include "core/base/observer.h"

#include "core/base/with_timestamp.h"
#include "core/inf/runnable.h"
#include "core/util/holder_util.h"

namespace ark {

namespace {

class Signal final : public Boolean, public Runnable {
public:
    Signal(bool value, bool signalValue)
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

Observer::Observer(bool oneshot)
    : _oneshot(oneshot)
{
}

void Observer::notify()
{
    std::vector<Callback> callbacks = std::move(_callbacks);
    for(const auto& [func, oneshot, owned] : callbacks)
    {
        func->run();
        if(!(oneshot || (owned && func.unique())))
            _callbacks.push_back({func, oneshot, owned});
    }
}

void Observer::addCallback(sp<Runnable> callback)
{
    ASSERT(callback);
    _callbacks.push_back({std::move(callback), _oneshot, false});
}

sp<Boolean> Observer::addBooleanSignal(bool value)
{
    sp<Signal> signal = sp<Signal>::make(value, !value);
    _callbacks.push_back({signal, _oneshot, true});
    return signal;
}

}
