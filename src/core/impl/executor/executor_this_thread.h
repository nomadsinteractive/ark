#ifndef ARK_CORE_IMPL_EXECUTOR_EXECUTOR_THIS_THREAD_H_
#define ARK_CORE_IMPL_EXECUTOR_EXECUTOR_THIS_THREAD_H_

#include "core/inf/executor.h"

namespace ark {

class ExecutorThisThread : public Executor {
public:

    virtual void execute(const sp<Runnable>& task) override;

};

}

#endif
