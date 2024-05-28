#pragma once

#include <vector>

#include "core/forwarding.h"

namespace ark {

class Wirable {
public:
    virtual ~Wirable() = default;

    virtual std::vector<std::pair<TypeId, Box>> onWire(const Traits& components) = 0;

};

}
