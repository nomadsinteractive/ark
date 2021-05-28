#include "core/impl/message_loop/message_loop_default.h"

#include "core/epi/disposed.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/executor/executor_this_thread.h"
#include "core/impl/runnable/runnable_composite.h"
#include "core/util/log.h"

namespace ark {

MessageLoopDefault::MessageLoopDefault(sp<Variable<uint64_t>> clock)
    : MessageLoopDefault(std::move(clock), sp<ExecutorThisThread>::make())
{
}

MessageLoopDefault::MessageLoopDefault(sp<Variable<uint64_t>> clock, sp<Executor> executor)
    : _clock(std::move(clock)), _executor(std::move(executor))
{
}

void MessageLoopDefault::post(const sp<Runnable>& task, float delay)
{
    DASSERT(task);
    _scheduled.push(Task(task, delay == 0 ? 0 : static_cast<uint64_t>(_clock->val() + delay * 1000000), 0));
}

void MessageLoopDefault::schedule(const sp<Runnable>& task, float interval)
{
    DASSERT(task);
    _scheduled.push(Task(task, 0, static_cast<uint32_t>(interval * 1000000)));
}

uint64_t MessageLoopDefault::pollOnce()
{
    uint64_t now = _clock->val();
    for(Task& task : _scheduled.clear())
        requestNextTask(std::move(task));

    std::vector<sp<Runnable>> scheduled;
    while(_tasks.size() > 0)
    {
        const Task& front = _tasks.front();
        if(front.nextFireTick() <= now)
        {
            Task nextTask = front;
            _tasks.pop_front();
            scheduled.push_back(nextTask.entry());
            if(nextTask.interval())
            {
                const sp<Disposed>& lifecycle = nextTask.disposed();
                if(!lifecycle || !lifecycle->val())
                {
                    nextTask.setNextFireTick(now + nextTask.interval());
                    requestNextTask(std::move(nextTask));
                }
            }
        }
        else
        {
            runScheduledTask(scheduled);
            return front.nextFireTick();
        }
    }
    runScheduledTask(scheduled);
    return now + 1000;
}

void MessageLoopDefault::requestNextTask(Task task)
{
    for(auto iter = _tasks.begin(); iter != _tasks.end(); ++iter)
    {
        const Task& i = (*iter);
        if(i.nextFireTick() >= task.nextFireTick())
        {
            _tasks.insert(iter, std::move(task));
            return;
        }
    }
    _tasks.push_back(std::move(task));
}

void MessageLoopDefault::runScheduledTask(std::vector<sp<Runnable>> scheduled)
{
    if(!scheduled.empty())
    {
        if(scheduled.size() == 1)
            _executor->execute(scheduled.at(0));
        else
            _executor->execute(sp<RunnableComposite>::make(std::move(scheduled)));
    }
}

MessageLoopDefault::Task::Task(const sp<Runnable>& entry, uint64_t nextFireTick, uint32_t interval)
    : _entry(entry), _disposed(entry.as<Disposed>()), _next_fire_tick(nextFireTick), _interval(interval)
{
}

const sp<Runnable>& MessageLoopDefault::Task::entry() const
{
    return _entry;
}

const sp<Disposed>& MessageLoopDefault::Task::disposed() const
{
    return _disposed;
}

uint64_t MessageLoopDefault::Task::nextFireTick() const
{
    return _next_fire_tick;
}

uint32_t MessageLoopDefault::Task::interval() const
{
    return _interval;
}

void MessageLoopDefault::Task::setNextFireTick(uint64_t tick)
{
    _next_fire_tick = tick;
}

}
