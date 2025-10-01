#pragma once

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

}

#if ARK_FLAG_BUILD_TYPE == 1
    #define  LOGD(...)     ark::Log::d(ark::Log::func(__ARK_FUNCTION__).c_str(), ark::Strings::sprintf(__VA_ARGS__).c_str())
#else
    #define  LOGD(...)
#endif
#define  LOGW(...)     ark::Log::w(ark::Log::func(__ARK_FUNCTION__).c_str(), ark::Strings::sprintf(__VA_ARGS__).c_str())
#define  LOGE(...)     ark::Log::e(ark::Log::func(__ARK_FUNCTION__).c_str(), ark::Strings::sprintf(__VA_ARGS__).c_str())
