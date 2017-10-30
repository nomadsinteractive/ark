#ifndef ARK_CORE_BASE_THREAD_POOL_EXECUTOR_H_
#define ARK_CORE_BASE_THREAD_POOL_EXECUTOR_H_

#include "core/base/thread.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ThreadPoolExecutor : public Executor {
public:
    ThreadPoolExecutor(uint32_t capacity = 0);

    virtual void execute(const sp<Runnable>& task) override;

private:
    class Worker {
    public:
        Worker(const sp<Thread::Stub>& stub);
        ~Worker();

        bool busy() const;
        void post(const sp<Runnable>& task) const;

        sp<Runnable> entry() const;

    private:
        class Stub : public Runnable {
        public:
            Stub(const sp<Thread::Stub>& threadStub);

            virtual void run() override;

            bool busy() const;
            void post(const sp<Runnable>& task);

        private:
            sp<Thread::Stub> _thread_stub;
            OCSQueue<sp<Runnable>> _pendings;
            bool _busy;
        };

    private:
        sp<Thread::Stub> _thread_stub;
        sp<Stub> _stub;
    };

    sp<Worker> createWorker();

private:
    LockFreeStack<sp<Worker>> _workers;
    uint32_t _capacity;
};


}
#endif
