#pragma once

#include "core/forwarding.h"

namespace ark {

template<typename T> class Dictionary {
public:
    virtual ~Dictionary() = default;

    virtual T get(const String& name) = 0;
};

}
