#include "core/base/api.h"

#include <cstdarg>
#include <system_error>

#include "core/util/strings.h"
#include "core/util/log.h"

namespace ark {

void __fatal__(const char* func, const char* condition, const char* message)
{
    const String str = Strings::sprintf("%s%s", condition ? Strings::sprintf("\"%s\" failed! ", condition).c_str() : "", message);
#if ARK_FLAG_DEBUG
    uint32_t raise = 1;
#else
    constexpr bool raise = true;
#endif
    Log::e(func, str.c_str());
    if(raise)
        throw std::runtime_error(str.c_str());
}

void __warning__(const char* func, const char* /*condition*/, const char* message)
{
    Log::w(func, message);
}

void __trace__(const char* func, const char* /*condition*/, const char* message)
{
    Log::d(func, ">>>__TRACING__>>>");
    Log::d(func, message);
    Log::d(func, "<<<__TRACING__<<<");
}

void __thread_check__(const char* func, const THREAD_ID threadId)
{
    THREAD_ID against = THREAD_ID_UNSPECIFIED;
    switch(threadId)
    {
        case THREAD_ID_UNSPECIFIED:
            against = _internal::ThreadFlag<THREAD_ID_UNSPECIFIED>::id();
        break;
        case THREAD_ID_MAIN:
            against = _internal::ThreadFlag<THREAD_ID_MAIN>::id();
        break;
        case THREAD_ID_CORE:
            against = _internal::ThreadFlag<THREAD_ID_CORE>::id();
        break;
        case THREAD_ID_RENDERER:
            against = _internal::ThreadFlag<THREAD_ID_RENDERER>::id();
        break;
    }
    if(against != threadId)
        __message__(__fatal__, func, "", "ThreadId check failed: %d, should be %d", against, threadId);
}

void __message__(fnTraceCallback callback, const char* func, const char* condition, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const String str = Strings::svprintf(format, args);
    va_end(args);

    callback(func, condition, str.c_str());
}

}
