#include "core/base/clock.h"

#include <cmath>

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_timeout.h"
#include "core/util/conversions.h"
#include "core/util/math.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

namespace {

class ClockDuration : public Numeric {
public:
    ClockDuration(const sp<Variable<uint64_t>>& ticker)
        : _ticker(ticker), _initial_ticket(0) {
    }

    virtual float val() override {
        if(!_initial_ticket) {
            _initial_ticket = _ticker->val();
            return 0;
        }
        uint64_t v = _ticker->val();
        DASSERT(v >= _initial_ticket);
        return (v - _initial_ticket) / 1000000.0f;
    }

    virtual bool update(uint64_t timestamp) override {
        return _ticker->update(timestamp);
    }

private:
    sp<Variable<uint64_t>> _ticker;
    uint64_t _initial_ticket;
};

}

class Clock::Ticker : public Variable<uint64_t> {
public:
    Ticker(const sp<Variable<uint64_t>>& delegate)
        : _delegate(delegate), _bypass(0), _paused(0) {
    }

    virtual uint64_t val() override {
        return _paused ? _paused : _delegate->val() - _bypass;
    }

    virtual bool update(uint64_t timestamp) override {
        return _paused ? false : _delegate->update(timestamp);
    }

    void setDelegate(const sp<Variable<uint64_t>>& delegate) {
        _delegate = delegate;
    }

    void pause() {
        if(!_paused)
            _paused = val();
    }

    void resume() {
        if(_paused != 0) {
            _bypass += (_delegate->val() - _bypass - _paused);
            _paused = 0;
        }
    }

private:
    sp<Variable<uint64_t>> _delegate;
    uint64_t _bypass;
    uint64_t _paused;
};

Clock::Interval::Interval(uint64_t usec)
    : _usec(usec)
{
}

Clock::Interval::Interval(const Clock::Interval& interval)
    : _usec(interval._usec)
{
}

float Clock::Interval::sec() const
{
    return _usec / 1000000.0f;
}

uint64_t Clock::Interval::usec() const
{
    return _usec;
}

Clock::Elapsing::Elapsing(const Clock& clock)
    : _ticker(clock._ticker), _last_tick(0)
{
}

Clock::Elapsing::Elapsing(const Clock::Elapsing& other)
    : _ticker(other._ticker), _last_tick(other._last_tick)
{
}

void Clock::Elapsing::reset()
{
    _last_tick = _ticker->val();
}

Clock::Interval Clock::Elapsing::elapsed()
{
    uint64_t tick = _ticker->val();
    uint64_t interval = _last_tick ? tick - _last_tick : 0;
    _last_tick = tick;
    return interval;
}

Clock::Clock()
    : Clock(Platform::getSteadyClock())
{
}

Clock::Clock(const sp<Variable<uint64_t>>& ticker)
    : _ticker(sp<Ticker>::make(ticker))
{
}

uint64_t Clock::val()
{
    return _ticker->val();
}

bool Clock::update(uint64_t timestamp)
{
    return _ticker->update(timestamp);
}

uint64_t Clock::tick() const
{
    return _ticker->val();
}

const sp<Variable<uint64_t>> Clock::ticker() const
{
    return _ticker;
}

void Clock::setTicker(const sp<Variable<uint64_t>>& ticker)
{
    _ticker->setDelegate(ticker);
}

sp<Numeric> Clock::duration() const
{
    return sp<ClockDuration>::make(_ticker);
}

sp<Numeric> Clock::durationUntil(const sp<Numeric>& until) const
{
    return Math::min(duration(), until);
}

sp<Boolean> Clock::timeout(float seconds) const
{
    return sp<BooleanByTimeout>::make(duration(), seconds);
}

void Clock::pause() const
{
    _ticker->pause();
}

void Clock::resume() const
{
    _ticker->resume();
}

template<> ARK_API Clock::Interval Conversions::to<String, Clock::Interval>(const String& val)
{
    size_t len = val.length();
    if(val.endsWith("ms"))
        return static_cast<uint64_t>(Strings::parse<float>(val.substr(0, len - 2)) * 1000.0f);
    else if(val.endsWith("us"))
        return static_cast<uint64_t>(Strings::parse<float>(val.substr(0, len - 2)));
    else if(val.endsWith("s"))
        return static_cast<uint64_t>(Strings::parse<float>(val.substr(0, len - 1)) * 1000000.0f);
    return static_cast<uint64_t>(Strings::parse<float>(val) * 1000000.0f);
}

}
