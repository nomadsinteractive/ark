#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Updatable {
public:
    virtual ~Updatable() = default;

    virtual bool update(uint64_t timestamp) = 0;
};

}
