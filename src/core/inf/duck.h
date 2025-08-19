#pragma once

#include "core/forwarding.h"

namespace ark {

template<typename T> class Duck {
public:
    virtual ~Duck() = default;

    virtual void to(sp<T>& inst) = 0;

};

}
