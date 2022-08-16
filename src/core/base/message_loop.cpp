#include "core/base/message_loop.h"

#include "core/ark.h"

#include "core/epi/disposed.h"
#include "core/base/future.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/executor/executor_this_thread.h"
#include "core/impl/runnable/runnable_composite.h"
#include "core/util/log.h"

namespace ark {

MessageLoop::MessageLoop(sp<Variable<uint64_t>> clock)
    : MessageLoop(std::move(clock), sp<ExecutorThisThread>::make())
{
}

MessageLoop::MessageLoop(sp<Variable<uint64_t>> clock, sp<Executor> executor)
    : _clock(std::move(clock)), _executor(std::move(executor))
{
}

void MessageLoop::post(sp<Runnable> runnable, float delay, sp<Future> future)
{
    DASSERT(runnable);
    _scheduled.push(sp<Task>::make(std::move(runnable), std::move(future), delay == 0 ? 0 : static_cast<uint64_t>(_clock->val() + delay * 1000000), 0));
}

sp<Future> MessageLoop::schedule(sp<Runnable> runnable, float interval, sp<Future> future)
{
    DASSERT(runnable);
    _scheduled.push(sp<Task>::make(std::move(runnable), std::move(future), 0, static_cast<uint32_t>(interval * 1000000)));
    return future;
}

uint64_t MessageLoop::pollOnce(uint64_t now)
{
    DPROFILER_TRACE("MessageLoop");
    if(now == 0)
        now = _clock->val();

    uint64_t nextFireTick = now + 10000;
    for(sp<Task>& task : _scheduled.clear())
        requestNextTask(std::move(task));

    std::vector<sp<Runnable>> scheduled;
    while(_tasks.size() > 0)
    {
        Task& front = _tasks.front();
        if(front.nextFireTick() <= now)
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
    }
    runScheduledTask(std::move(scheduled));
    return nextFireTick;
}

void MessageLoop::requestNextTask(sp<Task> task)
{
    for(auto iter = _tasks.begin(); iter != _tasks.end(); ++iter)
    {
        const sp<Task>& i = (*iter);
        if(i->nextFireTick() >= task->nextFireTick())
        {
            _tasks.insert(iter, std::move(task));
            return;
        }
    }
    _tasks.push_back(std::move(task));
}

void MessageLoop::runScheduledTask(std::vector<sp<Runnable>> scheduled)
{
    if(!scheduled.empty())
    {
        if(scheduled.size() == 1)
            _executor->execute(scheduled.at(0));
        else
            _executor->execute(sp<RunnableComposite>::make(std::move(scheduled)));
    }
}

MessageLoop::Task::Task(sp<Runnable> target, sp<Future> future, uint64_t nextFireTick, uint32_t interval)
    : _target(std::move(target)), _future(std::move(future)), _next_fire_tick(nextFireTick), _interval(interval)
{
}

void MessageLoop::Task::run()
{
    if(_future)
    {
        if(!_future->isCancelled())
        {
            _target->run();
            _future->done();
        }
    }
    else
        _target->run();
}

bool MessageLoop::Task::isCancelled() const
{
    return _future ? _future->isCancelled() : false;
}

uint64_t MessageLoop::Task::nextFireTick() const
{
    return _next_fire_tick;
}

uint32_t MessageLoop::Task::interval() const
{
    return _interval;
}

void MessageLoop::Task::setNextFireTick(uint64_t tick)
{
    _next_fire_tick = tick;
}

}
