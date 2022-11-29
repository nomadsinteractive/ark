#include "core/base/api.h"

#include <cstdarg>
#include <map>
#include <system_error>

#include "core/util/strings.h"
#include "core/util/log.h"

namespace ark {

static bool gTraceFlag = false;

void __fatal__(const char* func, const char* condition, const char* message)
{
    const String str = Strings::sprintf("%s%s", condition ? Strings::sprintf("\"%s\" failed! ", condition).c_str() : "", message);
    Log::e(func, str.c_str());
    throw std::logic_error(str.c_str());
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
    gTraceFlag = false;
}

bool __trace_flag__()
{
    return gTraceFlag;
}

void __set_trace_flag__()
{
    gTraceFlag = true;
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
