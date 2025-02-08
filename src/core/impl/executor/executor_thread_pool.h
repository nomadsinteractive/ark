#pragma once

#include "core/base/thread.h"
#include "core/concurrent/lf_stack.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/impl/executor/executor_worker_thread.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ExecutorThreadPool final : public Executor {
public:
    ExecutorThreadPool(sp<Executor> exceptionExecutor = nullptr, uint32_t capacity = 0);

    void execute(sp<Runnable> task) override;

    sp<ExecutorWorkerThread> obtainWorkerThread();

    void releaseAll(bool wait);

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

    sp<ExecutorWorkerThread> createWorkerThread();

private:
    sp<Stub> _stub;
};

}
