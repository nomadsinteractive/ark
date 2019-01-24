#ifndef ARK_CORE_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_DEFAULT_H_
#define ARK_CORE_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_DEFAULT_H_

#include <list>

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"
#include "core/forwarding.h"
#include "core/inf/message_loop.h"
#include "core/types/shared_ptr.h"


namespace ark {

class ARK_API MessageLoopDefault {
public:
    void post(const sp<Runnable>& task, uint64_t nextFireTick);
    void schedule(const sp<Runnable>& task, uint32_t interval);

    uint64_t pollOnce(uint64_t tick);

private:
    class Task {
    public:
        Task(const sp<Runnable>& entry, uint64_t nextFireTick, uint32_t interval);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Task);

        const sp<Runnable>& entry() const;
        const sp<Lifecycle>& expirable() const;

        uint64_t nextFireTick() const;
        uint32_t interval() const;
        void setNextFireTick(uint64_t tick);

    private:
        sp<Runnable> _entry;
        sp<Lifecycle> _lifecycle;
        uint64_t _next_fire_tick;
        uint32_t _interval;
    };

    void requestNextTask(Task task);

private:
    std::list<Task> _tasks;
    LFStack<Task> _scheduled;
};

}

#endif
