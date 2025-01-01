#pragma once


#include <chrono>

#include "core/base/api.h"
#include "core/util/strings.h"

namespace ark {

template<uint32_t Id, uint32_t Interval> class Trace {
public:
    Trace()
        : _start(std::chrono::steady_clock::now()) {
    }

    ~Trace() {
        uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start).count();
        if(elapsed > _TRACE_MAX)
            _TRACE_MAX = elapsed;
        if(_TRACE_MIN > elapsed)
            _TRACE_MIN = elapsed;
        _TOTAL_MS += elapsed;
        _TICK ++;
        if(_TICK == Interval) {
            Log::d(Strings::sprintf("Trace[%d]", Id).c_str(), Strings::sprintf("Average time: %lldms, min: %lld, max: %lld", _TOTAL_MS / Interval, _TRACE_MIN, _TRACE_MAX).c_str());
            _TOTAL_MS = 0;
            _TICK = 0;
            _TRACE_MAX = 0;
            _TRACE_MIN = std::numeric_limits<std::uint64_t>::max();
        }
    }

private:
    static uint32_t _TICK;
    static uint64_t _TOTAL_MS;
    static uint64_t _TRACE_MAX, _TRACE_MIN;

    std::chrono::time_point<std::chrono::steady_clock> _start;
};

template<uint32_t Id, uint32_t Interval> uint32_t Trace<Id, Interval>::_TICK = 0;
template<uint32_t Id, uint32_t Interval> uint64_t Trace<Id, Interval>::_TOTAL_MS = 0;
template<uint32_t Id, uint32_t Interval> uint64_t Trace<Id, Interval>::_TRACE_MAX = 0;
template<uint32_t Id, uint32_t Interval> uint64_t Trace<Id, Interval>::_TRACE_MIN = std::numeric_limits<std::uint64_t>::max();

}
