#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

class ARK_API Executor {
public:
    virtual ~Executor() = default;

    virtual void execute(sp<Runnable> task) = 0;
};

}
