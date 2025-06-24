#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

class ARK_API Executor {
public:
    virtual ~Executor() = default;

    virtual void execute(const sp<Runnable>& task) = 0;
};

}
