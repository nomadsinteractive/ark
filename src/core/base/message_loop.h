#ifndef ARK_CORE_BASE_MESSAGE_LOOP_H_
#define ARK_CORE_BASE_MESSAGE_LOOP_H_

#include <list>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API MessageLoop {
public:
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

    MessageLoop(sp<Variable<uint64_t>> clock);
    MessageLoop(sp<Variable<uint64_t>> clock, sp<Executor> executor);

//  [[script::bindings::auto]]
    void post(sp<Runnable> runnable, float delay, sp<Future> future = nullptr);
//  [[script::bindings::auto]]
    sp<Future> schedule(sp<Runnable> runnable, float interval, sp<Future> future = nullptr);

    uint64_t pollOnce();

private:
    class Task : public Runnable {
    public:
        Task(sp<Runnable> target, sp<Future> future, uint64_t nextFireTick, uint32_t interval);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Task);

        virtual void run() override;

        bool isCancelled() const;

        uint64_t nextFireTick() const;
        uint32_t interval() const;
        void setNextFireTick(uint64_t tick);

    private:
        sp<Runnable> _target;
        sp<Future> _future;
        uint64_t _next_fire_tick;
        uint32_t _interval;

    };

    void requestNextTask(sp<Task> task);
    void runScheduledTask(std::vector<sp<Runnable>> scheduled);

private:
    sp<Variable<uint64_t>> _clock;
    sp<Executor> _executor;

    std::list<sp<Task>> _tasks;
    LFStack<sp<Task>> _scheduled;

};

}

#endif
