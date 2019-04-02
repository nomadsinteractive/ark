#ifndef ARK_PLATFORM_ANY_PLATFORM_INDEPENDENT_H_
#define ARK_PLATFORM_ANY_PLATFORM_INDEPENDENT_H_

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API PlatformIndependent {
public:

    static bool isAbsolutePath(const String& path);
    static bool isDirectory(const String& path);
    static bool isFile(const String& path);

    static char dirSeparator();
    static char pathSeparator();
    static String pathJoin(const String& p1, const String& p2);

    static sp<Variable<uint64_t>> getSteadyClock();

protected:
    static char _DIR_SEPARATOR;
    static char _PATH_SEPARATOR;

};

}

#endif
