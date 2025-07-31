#include "core/base/message_loop.h"

#include "core/ark.h"

#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/executor/executor_this_thread.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/types/safe_var.h"

namespace ark {

class MessageLoop::Task final : public Runnable {
public:
    Task(sp<Runnable> runnable, sp<Boolean> canceled, const uint64_t nextFireTick, const uint64_t interval)
        : _runnable(std::move(runnable)), _canceled(std::move(canceled)), _next_fire_tick(nextFireTick), _interval(interval)
    {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Task);

    void run() override
    {
        if(!(_canceled && _canceled->val()))
            _runnable->run();
    }

    bool isCancelled() const
    {
        return _canceled && _canceled->val();
    }

    sp<Runnable> _runnable;
    sp<Boolean> _canceled;
    uint64_t _next_fire_tick;
    uint64_t _interval;
};

MessageLoop::MessageLoop(sp<Variable<uint64_t>> clock)
    : MessageLoop(std::move(clock), sp<ExecutorThisThread>::make())
{
}

MessageLoop::MessageLoop(sp<Variable<uint64_t>> clock, sp<Executor> executor)
    : _clock(std::move(clock)), _executor(std::move(executor))
{
}

void MessageLoop::post(sp<Runnable> runnable, const float delay, sp<Boolean> canceled)
{
    ASSERT(runnable);
    _scheduled.push(sp<Task>::make(std::move(runnable), std::move(canceled), delay == 0 ? 0 : _clock->val() + static_cast<uint64_t>(delay * 1000000), 0));
}

void MessageLoop::post(std::function<void()> task, const float delay, sp<Boolean> canceled)
{
    post(sp<Runnable>::make<RunnableByFunction>(std::move(task)), delay, std::move(canceled));
}

void MessageLoop::schedule(sp<Runnable> runnable, const float interval, sp<Boolean> canceled)
{
    ASSERT(runnable);
    const uint64_t intervalNs = interval * 1000000;
    _scheduled.push(sp<Task>::make(std::move(runnable), std::move(canceled), _clock->val() + intervalNs, intervalNs));
}

void MessageLoop::pollOnce()
{
    DPROFILER_TRACE("MessageLoop");
    const uint64_t now = _clock->val();

    for(sp<Task>& task : _scheduled.clear())
        requestNextTask(std::move(task));

    while(!_tasks.empty())
        if(const Task& front = *_tasks.front(); front._next_fire_tick <= now)
        {
            sp<Task> nextTask = _tasks.front();
            _tasks.pop_front();
            if(!nextTask->isCancelled())
            {
                if(nextTask->_interval)
                {
                    nextTask->_next_fire_tick = now + nextTask->_interval;
                    requestNextTask(nextTask);
                }
                _executor->execute(nextTask);
            }
        }
        else
            break;
}

void MessageLoop::requestNextTask(sp<Task> task)
{
    for(auto iter = _tasks.begin(); iter != _tasks.end(); ++iter)
        if(const sp<Task>& i = (*iter); i->_next_fire_tick > task->_next_fire_tick)
        {
            _tasks.insert(iter, std::move(task));
            return;
        }
    _tasks.push_back(std::move(task));
}

}
