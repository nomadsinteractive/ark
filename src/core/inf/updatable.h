#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Updatable {
public:
    virtual ~Updatable() = default;

    virtual bool update(uint32_t tick) = 0;
};

}
