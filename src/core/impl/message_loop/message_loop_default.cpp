#include "core/impl/message_loop/message_loop_default.h"

#include "core/epi/lifecycle.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

namespace ark {

void MessageLoopDefault::post(const sp<Runnable>& task, uint64_t nextFireTick)
{
    DASSERT(task);
    _scheduled.push(Task(task, nextFireTick, 0));
}

void MessageLoopDefault::schedule(const sp<Runnable>& task, uint32_t interval)
{
    DASSERT(task);
    _scheduled.push(Task(task, 0, interval));
}

uint64_t MessageLoopDefault::pollOnce(uint64_t tick)
{
    for(Task& task : _scheduled.clear())
        requestNextTask(std::move(task));

    while(_tasks.size() > 0)
    {
        const Task& front = _tasks.front();
        if(front.nextFireTick() <= tick)
        {
            Task nextTask = front;
            _tasks.pop_front();
            nextTask.entry()->run();
            if(nextTask.interval())
            {
                const sp<Lifecycle>& lifecycle = nextTask.expirable();
                if(!lifecycle || !lifecycle->isDisposed())
                {
                    nextTask.setNextFireTick(tick + nextTask.interval());
                    requestNextTask(std::move(nextTask));
                }
            }
        }
        else
            return front.nextFireTick();
    }
    return tick + 1000;
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

MessageLoopDefault::Task::Task(const sp<Runnable>& entry, uint64_t nextFireTick, uint32_t interval)
    : _entry(entry), _lifecycle(entry.as<Lifecycle>()), _next_fire_tick(nextFireTick), _interval(interval)
{
}

const sp<Runnable>& MessageLoopDefault::Task::entry() const
{
    return _entry;
}

const sp<Lifecycle>& MessageLoopDefault::Task::expirable() const
{
    return _lifecycle;
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
