#include "core/util/log.h"

#include "platform/platform.h"

namespace ark {

void Log::d(const char* tag, const char* content)
{
#if ARK_FLAG_DEBUG
    log(LOG_LEVEL_DEBUG, tag, content);
#endif
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
    Platform::log(logLevel, tag, content);
}

String Log::func(const String& f)
{
    String::size_type pos = f.find('(');
    return pos != String::npos ? f.substr(0, pos) : f;
}

}
