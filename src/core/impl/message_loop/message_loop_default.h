#ifndef ARK_CORE_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_DEFAULT_H_
#define ARK_CORE_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_DEFAULT_H_

#include <list>

#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/forwarding.h"
#include "core/inf/message_loop.h"
#include "core/types/shared_ptr.h"


namespace ark {

class ARK_API MessageLoopDefault : public MessageLoop {
public:
    MessageLoopDefault(const sp<Variable<uint64_t>>& ticker);

    virtual void post(const sp<Runnable>& task, float delay) override;
    virtual void schedule(const sp<Runnable>& task, float interval) override;

    virtual uint64_t pollOnce() override;

private:
    class Task {
    public:
        Task(const sp<Runnable>& entry, uint64_t nextFireTick, uint32_t interval);
        Task(const Task& task);

        const sp<Runnable>& entry() const;
        const sp<Lifecycle>& expirable() const;

        uint64_t nextFireTick() const;
        uint32_t interval() const;
        void setNextFireTick(uint64_t tick);

    private:
        sp<Runnable> _entry;
        sp<Lifecycle> _expirable;
        uint64_t _next_fire_tick;
        uint32_t _interval;
    };

    void requestNextTask(const Task& task);

private:
    sp<Variable<uint64_t>> _ticker;
    std::list<Task> _tasks;
    LockFreeStack<Task> _scheduled;
};

}

#endif
