#pragma once

#include "core/forwarding.h"
#include "core/base/thread.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/concurrent/lf_queue.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ExecutorWorkerThread final : public Executor {
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
    ExecutorWorkerThread(sp<Strategy> strategy, String name);
    ~ExecutorWorkerThread() override;

    void execute(sp<Runnable> task) override;
    void terminate() const;
    void tryJoin() const;

    const sp<Strategy>& strategy() const;

private:
    class Worker final : public Runnable {
    public:
        Worker(Thread thread, sp<Strategy> strategy);

        void run() override;

        Thread _thread;
        sp<Strategy> _strategy;

        LFQueue<sp<Runnable>> _pending_tasks;
    };

    class WaitPredicate {
    public:
        WaitPredicate(const TimePoint& until);

        bool operator() () const;

    private:
        TimePoint _until;
    };

private:
    Thread _thread;
    sp<Worker> _worker;
};

}
