#ifndef ARK_CORE_UTIL_LOG_H_
#define ARK_CORE_UTIL_LOG_H_

#include <chrono>

#include "core/base/api.h"
#include "core/util/strings.h"

namespace ark {

class ARK_API Log {
public:
    enum LogLevel {
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_WARNING,
        LOG_LEVEL_ERROR
    };

    static void d(const char* tag, const char* content);
    static void w(const char* tag, const char* content);
    static void e(const char* tag, const char* content);

    static void log(LogLevel logLevel, const char* tag, const char* content);

    static String func(const String& f);
};

template<uint32_t Id, uint32_t Interval> class Trace {
public:
    Trace()
        : _start(std::chrono::steady_clock::now()) {
    }

    ~Trace() {
        uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start).count();
        if(elapsed > _MAX)
            _MAX = elapsed;
        if(_MIN > elapsed)
            _MIN = elapsed;
        _TOTAL_MS += elapsed;
        _TICK ++;
        if(_TICK == Interval) {
            Log::d(Strings::sprintf("Trace[%d]", Id).c_str(), Strings::sprintf("Average time: %lldms, min: %lld, max: %lld", _TOTAL_MS / Interval, _MIN, _MAX).c_str());
            _TOTAL_MS = 0;
            _TICK = 0;
            _MAX = 0;
            _MIN = std::numeric_limits<std::uint64_t>::max();
        }
    }

private:
    static uint32_t _TICK;
    static uint64_t _TOTAL_MS;
    static uint64_t _MAX, _MIN;

    std::chrono::time_point<std::chrono::steady_clock> _start;
};

template<uint32_t Id, uint32_t Interval> uint32_t Trace<Id, Interval>::_TICK = 0;
template<uint32_t Id, uint32_t Interval> uint64_t Trace<Id, Interval>::_TOTAL_MS = 0;
template<uint32_t Id, uint32_t Interval> uint64_t Trace<Id, Interval>::_MAX = 0;
template<uint32_t Id, uint32_t Interval> uint64_t Trace<Id, Interval>::_MIN = std::numeric_limits<std::uint64_t>::max();

}
#ifdef ARK_FLAG_DEBUG
    #define  LOGD(...)     ark::Log::d(ark::Log::func(__ARK_FUNCTION__).c_str(), ark::Strings::sprintf(__VA_ARGS__).c_str())
#else
    #define  LOGD(...)
#endif
#define  LOGW(...)     ark::Log::w(ark::Log::func(__ARK_FUNCTION__).c_str(), ark::Strings::sprintf(__VA_ARGS__).c_str())
#define  LOGE(...)     ark::Log::e(ark::Log::func(__ARK_FUNCTION__).c_str(), ark::Strings::sprintf(__VA_ARGS__).c_str())

#endif
