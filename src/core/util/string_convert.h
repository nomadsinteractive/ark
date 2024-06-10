#pragma once

#include "core/forwarding.h"

namespace ark {

class StringConvert {
public:

    template<typename T> static String repr(const T& obj);
    template<typename T> static T eval(const String& repr);
};

}
