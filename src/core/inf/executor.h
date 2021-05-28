#ifndef ARK_CORE_INF_EXECUTOR_H_
#define ARK_CORE_INF_EXECUTOR_H_

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

class ARK_API Executor {
public:
    virtual ~Executor() = default;

    virtual void execute(const sp<Runnable>& task) = 0;
};

}

#endif
