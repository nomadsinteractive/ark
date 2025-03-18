#include "core/impl/executor/executor_thread_pool.h"

#include "core/base/string.h"

namespace ark {

namespace {

class ThrowException final : public Runnable {
public:
    ThrowException(String what)
        : _what(std::move(what)) {
    }

    void run() override {
        FATAL("Unhanlded exception in thread: %s", _what.c_str());
    }

private:
    String _what;
};

}

class ExecutorThreadPool::WorkerThreadStrategy final : public ExecutorWorkerThread::Strategy {
public:
    WorkerThreadStrategy(const sp<Stub>& stub)
        : _stub(stub), _idled_cycle(0)
    {
    }

    void onStart() override
    {
    }

    void onExit() override
    {
        bool removed = false;
        const volatile std::scoped_lock<std::mutex> guard(_stub->_mutex);
        for(const sp<ExecutorWorkerThread>& i : _stub->_worker_threads.clear())
            if(i->strategy().get() != this)
                _stub->_worker_threads.push(i);
            else
                removed = true;

        DCHECK(removed, "Unable to remove Worker: %p", this);
        DCHECK(_stub->_worker_count > 0, "Worker count mismatch");
         -- _stub->_worker_count;
    }

    uint64_t onBusy() override
    {
        _idled_cycle = 0;
        return 0;
    }

    uint64_t onIdle(Thread& thread) override
    {
        _idled_cycle ++;
        if(_idled_cycle > 20000 && _stub->_worker_count.load(std::memory_order_relaxed) > _stub->_capacity)
            thread.terminate();
        return 20000;
    }

    void onException(const std::exception& e) override
    {
        if(_stub->_exception_executor)
            _stub->_exception_executor->execute(sp<ThrowException>::make(e.what()));
    }

    bool isIdle() const
    {
        return _idled_cycle > 0;
    }

    void markBusy()
    {
        _idled_cycle = 0;
    }

private:
    sp<Stub> _stub;
    uint32_t _idled_cycle;
};

ExecutorThreadPool::ExecutorThreadPool(sp<Executor> exceptionExecutor, const uint32_t capacity)
    : _stub(sp<Stub>::make(std::move(exceptionExecutor), std::max<uint32_t>(2, capacity ? capacity : std::thread::hardware_concurrency())))
{
}

void ExecutorThreadPool::execute(sp<Runnable> task)
{
    obtainWorkerThread()->execute(std::move(task));
}

sp<ExecutorWorkerThread> ExecutorThreadPool::obtainWorkerThread()
{
    for(const sp<ExecutorWorkerThread>& i : _stub->_worker_threads)
        if(const sp<WorkerThreadStrategy>& strategy = i->strategy(); strategy->isIdle())
        {
            strategy->markBusy();
            return i;
        }

    return createWorkerThread();
}

void ExecutorThreadPool::releaseAll(const bool wait) const
{
    Vector<sp<ExecutorWorkerThread>> waitThreads;
    for(const sp<ExecutorWorkerThread>& i : _stub->_worker_threads)
        waitThreads.push_back(i);

    for(const sp<ExecutorWorkerThread>& i : waitThreads)
        i->terminate();

    if(wait)
        for(const sp<ExecutorWorkerThread>& i : waitThreads)
            i->tryJoin();
}

sp<ExecutorWorkerThread> ExecutorThreadPool::createWorkerThread()
{
    const sp<ExecutorWorkerThread> workerThread = sp<ExecutorWorkerThread>::make(sp<WorkerThreadStrategy>::make(_stub), "Worker");
    _stub->_worker_threads.push(workerThread);
    ++ _stub->_worker_count;
    return workerThread;
}

ExecutorThreadPool::Stub::Stub(sp<Executor> exceptionExecutor, uint32_t capacity)
    : _exception_executor(std::move(exceptionExecutor)), _capacity(capacity), _worker_count(0)
{
}

}
