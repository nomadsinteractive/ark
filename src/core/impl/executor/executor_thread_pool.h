#ifndef ARK_CORE_IMPL_EXECUTOR_EXECUTOR_THREAD_POOL_H_
#define ARK_CORE_IMPL_EXECUTOR_EXECUTOR_THREAD_POOL_H_

#include "core/base/thread.h"
#include "core/concurrent/lf_stack.h"
#include "core/concurrent/lf_queue.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/impl/executor/executor_worker_thread.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ExecutorThreadPool final : public Executor {
public:
    ExecutorThreadPool(const sp<MessageLoop>& messageLoop = nullptr, uint32_t capacity = 0);

    virtual void execute(const sp<Runnable>& task) override;

private:
    class WorkerThreadStrategy;

    struct Stub {
        Stub(const sp<MessageLoop>& messageLoop, uint32_t capacity);

        sp<MessageLoop> _message_loop;
        uint32_t _capacity;
        std::atomic<uint32_t> _worker_count;

        LFStack<sp<ExecutorWorkerThread>> _worker_threads;
        std::mutex _mutex;
    };

    class WorkerThreadStrategy : public ExecutorWorkerThread::Strategy {
    public:
        WorkerThreadStrategy(const sp<Stub>& stub);

        virtual void onStart() override;
        virtual void onExit() override;

        virtual uint64_t onBusy() override;
        virtual uint64_t onIdle(Thread& thread) override;

        virtual void onException(const std::exception& e) override;


        bool idle() const;

    private:
        sp<Stub> _stub;

        bool _idle;
        uint32_t _idled_cycle;

    };

    sp<ExecutorWorkerThread> createWorkerThread();

private:
    sp<Stub> _stub;
};


}
#endif
