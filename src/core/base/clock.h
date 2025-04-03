#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Clock final : public Variable<uint64_t> {
public:
    class ARK_API Interval {
    public:
        Interval(uint64_t usec);
        Interval(const Interval& interval);

        float sec() const;
        uint64_t usec() const;

    private:
        uint64_t _usec;
    };

    class ARK_API Elapsing {
    public:
        Elapsing(const Clock& clock);
        Elapsing(const Elapsing& other);

        void reset();
        Interval elapsed();

    private:
        sp<Variable<uint64_t>> _ticker;
        uint64_t _last_tick;
    };

public:
//  [[script::bindings::auto]]
    Clock();
    Clock(sp<Variable<uint64_t>> ticker);
    Clock(const Clock& other) = default;

    uint64_t val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::property]]
    uint64_t tick() const;

    sp<Variable<uint64_t>> ticker() const;
    void setTicker(const sp<Variable<uint64_t>>& ticker);

//  [[script::bindings::auto]]
    sp<Numeric> duration() const;
//  [[script::bindings::auto]]
    sp<Numeric> durationUntil(const sp<Numeric>& until) const;

//  [[script::bindings::auto]]
    sp<Boolean> timeout(float seconds) const;

//  [[script::bindings::auto]]
    void pause() const;
//  [[script::bindings::auto]]
    void resume() const;

private:
    class Ticker;

private:
    sp<Ticker> _ticker;

};

}
