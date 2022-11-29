#include "core/util/log.h"

#include "platform/platform.h"

#include "spdlog/spdlog.h"

namespace ark {

namespace {

class SpdLogInitializer {
public:
    SpdLogInitializer() {
        spdlog::set_pattern("[%f] <%^%L%$> %v");
        spdlog::flush_on(spdlog::level::warn);
        spdlog::set_level(spdlog::level::debug);
    }

};

}

void Log::d(const char* tag, const char* content)
{
    log(LOG_LEVEL_DEBUG, tag, content);
}

void Log::w(const char* tag, const char* content)
{
    log(LOG_LEVEL_WARNING, tag, content);
}

void Log::e(const char* tag, const char* content)
{
    log(LOG_LEVEL_ERROR, tag, content);
}

void Log::log(LogLevel logLevel, const char* tag, const char* content)
{
    static volatile SpdLogInitializer _spd_initialized;
    switch(logLevel) {
        case LOG_LEVEL_DEBUG:
            spdlog::debug("{} {}", tag, content);
            break;
        case LOG_LEVEL_WARNING:
            spdlog::warn("{} {}", tag, content);
            break;
        case LOG_LEVEL_ERROR:
            spdlog::error("{} {}", tag, content);
            break;
    }
//    Platform::log(logLevel, tag, content);
}

String Log::func(const String& f)
{
    String::size_type pos = f.find('(');
    return pos != String::npos ? f.substr(0, pos) : f;
}

}
