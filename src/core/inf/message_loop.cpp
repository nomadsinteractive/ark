#include "core/inf/message_loop.h"

#include "core/impl/runnable/runnable_by_function.h"
#include "core/impl/runnable/runnable_by_function_with_expired.h"

namespace ark {

void MessageLoop::postTask(std::function<void()> task, float delay)
{
    post(sp<RunnableByFunction>::make(std::move(task)), delay);
}

void MessageLoop::scheduleTask(std::function<bool()> task, float interval)
{
    schedule(sp<RunnableByFunctionWithExpired>::make(std::move(task)), interval);
}

}
