#include "core/base/message_loop.h"

#include "core/ark.h"

#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/executor/executor_this_thread.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/impl/runnable/runnable_composite.h"

namespace ark {

class MessageLoop::Task final : public Runnable {
public:
    Task(sp<Runnable> target, sp<Boolean> canceled, uint64_t nextFireTick, uint32_t interval)
        : _target(std::move(target)), _canceled(std::move(canceled)), _next_fire_tick(nextFireTick), _interval(interval)
    {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Task);

    void run() override
    {
        if(_canceled)
        {
            if(!_canceled->val())
                _target->run();
        }
        else
            _target->run();
    }

    bool isCancelled() const
    {
        return _canceled ? _canceled->val() : false;
    }

    uint64_t nextFireTick() const
    {
        return _next_fire_tick;
    }
    uint32_t interval() const
    {
        return _interval;
    }

    void setNextFireTick(uint64_t tick)
    {
        _next_fire_tick = tick;
    }

private:
    sp<Runnable> _target;
    sp<Boolean> _canceled;
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

void MessageLoop::post(std::function<void()> task, float delay, sp<Boolean> canceled)
{
    post(sp<RunnableByFunction>::make(std::move(task)), delay, std::move(canceled));
}

void MessageLoop::schedule(sp<Runnable> runnable, float interval, sp<Boolean> canceled)
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

    std::vector<sp<Runnable>> scheduled;
    while(_tasks.size() > 0)
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
                scheduled.push_back(std::move(nextTask));
            }
        }
        else
        {
            nextFireTick = front.nextFireTick();
            break;
        }
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

void MessageLoop::runScheduledTask(std::vector<sp<Runnable>> scheduled) const
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
