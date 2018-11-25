#ifndef ARK_CORE_BASE_THREAD_POOL_EXECUTOR_H_
#define ARK_CORE_BASE_THREAD_POOL_EXECUTOR_H_

#include "core/base/thread.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ThreadPoolExecutor final : public Executor {
public:
    ThreadPoolExecutor(const sp<MessageLoop>& messageLoop = nullptr, uint32_t capacity = 0);

    virtual void execute(const sp<Runnable>& task) override;

private:
    class Worker;

    struct Stub {
        Stub(const sp<MessageLoop>& messageLoop, uint32_t capacity);

        sp<MessageLoop> _message_loop;
        uint32_t _capacity;
        std::atomic<uint32_t> _worker_count;

        LockFreeStack<sp<Worker>> _workers;
        std::mutex _mutex;
    };

    class Worker : public Runnable {
    public:
        Worker(const sp<Stub>& stub, const sp<Thread::Stub>& threadStub);
        ~Worker() override;

        bool idle() const;
        void post(const sp<Runnable>& task);

        virtual void run() override;

    private:
        void removeSelf();

    private:
        sp<Stub> _stub;
        sp<Thread::Stub> _thread_stub;

        OCSQueue<sp<Runnable>> _pendings;
        bool _idle;
        uint32_t _idled_cycle;
    };

    sp<Worker> createWorker();

private:
    sp<Stub> _stub;
};


}
#endif
