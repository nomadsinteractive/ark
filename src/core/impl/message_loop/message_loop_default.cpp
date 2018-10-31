#include "core/impl/message_loop/message_loop_default.h"

#include "core/epi/lifecycle.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

namespace ark {

MessageLoopDefault::MessageLoopDefault(const sp<Variable<uint64_t>>& ticker)
    : _ticker(ticker)
{
}

void MessageLoopDefault::post(const sp<Runnable>& task, float delay)
{
    DASSERT(task);
    if(delay != 0)
    {
        uint64_t tick = _ticker->val();
        uint64_t l = static_cast<uint64_t>(delay * 1000000);
        _scheduled.push(Task(task, tick + l, 0));
    }
    else
        _scheduled.push(Task(task, 0, 0));
}

void MessageLoopDefault::schedule(const sp<Runnable>& task, float interval)
{
    DASSERT(task);
    _scheduled.push(Task(task, 0, (uint32_t) (interval * 1000000)));
}

uint64_t MessageLoopDefault::pollOnce()
{
    for(const Task& task : _scheduled.clear())
        requestNextTask(task);

    uint64_t tick = _ticker->val();
    while(_tasks.size() > 0)
    {
        const Task& front = _tasks.front();
        if(front.nextFireTick() <= tick)
        {
            Task nextTask = front;
            _tasks.pop_front();
            if(nextTask.interval())
            {
                const sp<Lifecycle>& expirable = nextTask.expirable();
                if(!expirable || !expirable->isDisposed())
                {
                    nextTask.setNextFireTick(tick + nextTask.interval());
                    requestNextTask(nextTask);
                }
            }
            nextTask.entry()->run();
            tick = _ticker->val();
        }
        else
        {
            return front.nextFireTick() > tick ? front.nextFireTick() - tick : 0;
        }
    }
    return 0;
}

void MessageLoopDefault::requestNextTask(const Task& task)
{
    for(auto iter = _tasks.begin(); iter != _tasks.end(); ++iter)
    {
        const Task& i = (*iter);
        if(i.nextFireTick() >= task.nextFireTick())
        {
            _tasks.insert(iter, task);
            return;
        }
    }
    _tasks.push_back(task);
}

MessageLoopDefault::Task::Task(const sp<Runnable>& entry, uint64_t nextFireTick, uint32_t interval)
    : _entry(entry), _expirable(entry.as<Lifecycle>()), _next_fire_tick(nextFireTick), _interval(interval)
{
}

MessageLoopDefault::Task::Task(const MessageLoopDefault::Task& task)
    : _entry(task._entry), _expirable(task._expirable), _next_fire_tick(task._next_fire_tick), _interval(task._interval)
{
}

const sp<Runnable>& MessageLoopDefault::Task::entry() const
{
    return _entry;
}

const sp<Lifecycle>& MessageLoopDefault::Task::expirable() const
{
    return _expirable;
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
