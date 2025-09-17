#include "core/base/api.h"

#include <cstdarg>
#include <system_error>

#include "core/util/strings.h"
#include "core/util/log.h"

namespace ark {

static std::thread::id _thread_ids[THREAD_NAME_ID_COUNT];

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

bool __thread_check__(const THREAD_NAME_ID threadNameId)
{
    ASSERT(threadNameId != THREAD_NAME_ID_UNSPECIFIED);
    return _thread_ids[threadNameId] == std::this_thread::get_id();
}

void __thread_check__(const char* func, const THREAD_NAME_ID threadId)
{
    if(_thread_ids[threadId] != std::this_thread::get_id())
        __message__(__fatal__, func, "", "ThreadId check failed: %d, should be %d", threadId);
}

void __thread_init__(const THREAD_NAME_ID nameId)
{
    _thread_ids[nameId] = std::this_thread::get_id();
}

void __message__(const fnTraceCallback callback, const char* func, const char* condition, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const String str = Strings::svprintf(format, args);
    va_end(args);

    callback(func, condition, str.c_str());
}

}
