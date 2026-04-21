#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Runnable {
public:
    virtual ~Runnable() = default;

    virtual void run() = 0;
};

}
