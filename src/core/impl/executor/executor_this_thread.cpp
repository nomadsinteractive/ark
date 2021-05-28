#include "core/impl/executor/executor_this_thread.h"

#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

void ExecutorThisThread::execute(const sp<Runnable>& task)
{
    task->run();
}

}
