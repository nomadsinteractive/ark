#pragma once

#include "core/forwarding.h"

namespace ark {

class Wirable {
public:
    virtual ~Wirable() = default;

    virtual void onWire(const Traits& components) = 0;

};

}
