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
    ExecutorThreadPool(sp<Executor> exceptionExecutor = nullptr, uint32_t capacity = 0);

    virtual void execute(const sp<Runnable>& task) override;

    sp<ExecutorWorkerThread> obtainWorkerThread();

private:
    class WorkerThreadStrategy;

    struct Stub {
        Stub(sp<Executor> exceptionExecutor, uint32_t capacity);

        sp<Executor> _exception_executor;
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

        bool isIdle() const;
        void markBusy();

    private:
        sp<Stub> _stub;

        uint32_t _idled_cycle;
    };

    sp<ExecutorWorkerThread> createWorkerThread();

private:
    sp<Stub> _stub;
};


}
#endif
