#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Runnable {
public:
    virtual ~Runnable() = default;

//  [[script::bindings::auto]]
    virtual void run() = 0;
};

}
