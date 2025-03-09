#include "core/base/message_loop.h"

#include "core/ark.h"

#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/executor/executor_this_thread.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/impl/runnable/runnable_composite.h"
#include "core/types/safe_var.h"

namespace ark {

class MessageLoop::Task final : public Runnable {
public:
    Task(sp<Runnable> target, sp<Boolean> canceled, const uint64_t nextFireTick, const uint32_t interval)
        : _runnable(std::move(target)), _canceled(std::move(canceled), false), _next_fire_tick(nextFireTick), _interval(interval)
    {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Task);

    void run() override
    {
        if(!_canceled.val())
            _runnable->run();
    }

    bool isCancelled() const
    {
        return _canceled.val();
    }

    uint64_t nextFireTick() const
    {
        return _next_fire_tick;
    }

    uint32_t interval() const
    {
        return _interval;
    }

    void setNextFireTick(const uint64_t tick)
    {
        _next_fire_tick = tick;
    }

private:
    sp<Runnable> _runnable;
    SafeVar<Boolean> _canceled;
    uint64_t _next_fire_tick;
    uint32_t _interval;
};

MessageLoop::MessageLoop(sp<Variable<uint64_t>> clock)
    : MessageLoop(std::move(clock), sp<ExecutorThisThread>::make())
{
}

MessageLoop::MessageLoop(sp<Variable<uint64_t>> clock, sp<Executor> executor)
    : _clock(std::move(clock)), _executor(std::move(executor))
{
}

void MessageLoop::post(sp<Runnable> runnable, float delay, sp<Boolean> canceled)
{
    ASSERT(runnable);
    _scheduled.push(sp<Task>::make(std::move(runnable), std::move(canceled), delay == 0 ? 0 : _clock->val() + static_cast<uint64_t>(delay * 1000000), 0));
}

void MessageLoop::post(std::function<void()> task, const float delay, sp<Boolean> canceled)
{
    post(sp<RunnableByFunction>::make(std::move(task)), delay, std::move(canceled));
}

void MessageLoop::schedule(sp<Runnable> runnable, const float interval, sp<Boolean> canceled)
{
    ASSERT(runnable);
    _scheduled.push(sp<Task>::make(std::move(runnable), std::move(canceled), 0, static_cast<uint32_t>(interval * 1000000)));
}

uint64_t MessageLoop::pollOnce()
{
    DPROFILER_TRACE("MessageLoop");
    const uint64_t now = _clock->val();

    uint64_t nextFireTick = now + 10000;
    for(sp<Task>& task : _scheduled.clear())
        requestNextTask(std::move(task));

    Vector<sp<Runnable>> scheduled;
    while(!_tasks.empty())
        if(Task& front = _tasks.front(); front.nextFireTick() <= now)
        {
            sp<Task> nextTask = _tasks.front();
            _tasks.pop_front();
            if(!nextTask->isCancelled())
            {
                if(nextTask->interval())
                {
                    nextTask->setNextFireTick(now + nextTask->interval());
                    requestNextTask(nextTask);
                }
                scheduled.emplace_back(nextTask);
            }
        }
        else
        {
            nextFireTick = front.nextFireTick();
            break;
        }
    if(!scheduled.empty())
        runScheduledTask(std::move(scheduled));
    return nextFireTick;
}

void MessageLoop::requestNextTask(sp<Task> task)
{
    for(auto iter = _tasks.begin(); iter != _tasks.end(); ++iter)
    {
        const sp<Task>& i = (*iter);
        if(i->nextFireTick() > task->nextFireTick())
        {
            _tasks.insert(iter, std::move(task));
            return;
        }
    }
    _tasks.push_back(std::move(task));
}

void MessageLoop::runScheduledTask(Vector<sp<Runnable>> scheduled) const
{
    if(!scheduled.empty())
    {
        if(scheduled.size() == 1)
            _executor->execute(scheduled.at(0));
        else
            _executor->execute(sp<RunnableComposite>::make(std::move(scheduled)));
    }
}

}
