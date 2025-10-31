#include "core/impl/executor/executor_scheduled.h"

namespace ark {

void ExecutorScheduled::execute(const sp<Runnable>& task)
{
    _queue.push(task);
}

void ExecutorScheduled::run()
{
    while(const Optional<sp<Runnable>>& opt = _queue.pop())
        opt.value()->run();
}

}
