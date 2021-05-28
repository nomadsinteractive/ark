#ifndef ARK_CORE_IMPL_EXECUTOR_WORKER_THREAD_H_
#define ARK_CORE_IMPL_EXECUTOR_WORKER_THREAD_H_

#include "core/forwarding.h"
#include "core/base/thread.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/concurrent/lf_queue.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ExecutorWorkerThread : public Executor {
public:
    class Strategy {
    public:
        virtual ~Strategy() = default;

        virtual void onStart() = 0;
        virtual void onExit() = 0;

        virtual uint64_t onBusy() = 0;
        virtual uint64_t onIdle(Thread& thread) = 0;

        virtual void onException(const std::exception& e) = 0;

    };

public:
    ExecutorWorkerThread(sp<Strategy> strategy);

    virtual void execute(const sp<Runnable>& task) override;

    const sp<Strategy>& strategy() const;

private:
    class Worker : public Runnable {
    public:
        Worker(Thread thread, sp<Strategy> strategy);

        virtual void run() override;

        Thread _thread;
        sp<Strategy> _strategy;

        LFQueue<sp<Runnable>> _pending_tasks;
    };

private:
    Thread _thread;
    sp<Worker> _worker;
};

}

#endif
