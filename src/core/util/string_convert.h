#ifndef ARK_CORE_TYPES_STRING_CONVERT_H_
#define ARK_CORE_TYPES_STRING_CONVERT_H_

#include "core/forwarding.h"

namespace ark {

class StringConvert {
public:
    template<typename F, typename T> static T to(const F& instance);

    template<typename T> static String repr(T& obj);
    template<typename T> static T str(const String& repr);
};

}

#endif
