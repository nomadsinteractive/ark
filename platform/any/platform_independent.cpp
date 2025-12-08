#include "platform/any/platform_independent.h"

#include <cctype>
#include <chrono>
#include <filesystem>

#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/util/strings.h"

#include "core/types/shared_ptr.h"

namespace ark {

namespace {

class PlatformSteadyClock final : public Variable<uint64_t> {
public:
    uint64_t val() override
    {
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    bool update(uint32_t /*tick*/) override
    {
        return true;
    }
};

}

bool PlatformIndependent::isFile(const String& filepath)
{
    return std::filesystem::is_regular_file(filepath.c_str());
}

bool PlatformIndependent::isAbsolutePath(const String& path)
{
    if(path.empty())
        return false;
    const char s = path.at(0);
    if(s == _DIR_SEPARATOR)
        return true;
    return path.length() >= 2 && std::isalpha(s) && path.at(1) == ':';
}

bool PlatformIndependent::isDirectory(const String& filepath)
{
    return std::filesystem::is_directory(filepath.c_str());
}


String PlatformIndependent::getRelativePath(const StringView path)
{
    return std::filesystem::path(path).relative_path().string();
}

char PlatformIndependent::dirSeparator()
{
    return _DIR_SEPARATOR;
}

char PlatformIndependent::pathSeparator()
{
    return _PATH_SEPARATOR;
}

String PlatformIndependent::pathJoin(const String& p1, const String& p2)
{
    if(!p1)
        return p2;
    if(!p2)
        return p1;

    const bool endsWithSeparator = p1.at(p1.length() - 1) == _DIR_SEPARATOR || p1.at(p1.length() - 1) == '/';
    const bool startsWithSeparator = p2 && (p2.at(0) == _DIR_SEPARATOR || p2.at(0) == '/');
    if(endsWithSeparator)
        return startsWithSeparator ? p1 + p2.substr(1) : p1 + p2;
    return startsWithSeparator ? p1 + p2 : Strings::sprintf("%s%c%s", p1.c_str(), _DIR_SEPARATOR, p2.c_str());
}

sp<Variable<uint64_t>> PlatformIndependent::getSteadyClock()
{
    return sp<Variable<uint64_t>>::make<PlatformSteadyClock>();
}

}
