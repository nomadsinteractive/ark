#include "core/base/api.h"

#include <cstdarg>
#include <map>
#include <system_error>

#include "core/util/strings.h"
#include "core/util/log.h"

namespace ark {

void __fatal__(const char* func, const char* condition, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const String str = condition ? Strings::sprintf("\"%s\" failed! ", condition) + Strings::svprintf(format, args) : Strings::svprintf(format, args);
    va_end(args);
    Log::e(func, str.c_str());
    throw std::logic_error(str.c_str());
}

void __warning__(const char* func, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const String str = Strings::svprintf(format, args);
    va_end(args);
    Log::w(func, str.c_str());
}

void __trace__()
{
     LOGD(">>>__TRACE HERE___<<<");
}

}
