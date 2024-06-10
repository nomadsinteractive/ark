#include "core/impl/executor/executor_thread_pool.h"

#include "core/base/string.h"

namespace ark {

namespace {

class ThrowException : public Runnable {
public:
    ThrowException(String what)
        : _what(std::move(what)) {
    }

    virtual void run() override {
        FATAL("Unhanlded exception in thread: %s", _what.c_str());
    }

private:
    String _what;
};

}

ExecutorThreadPool::ExecutorThreadPool(sp<Executor> exceptionExecutor, uint32_t capacity)
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
    {
        const sp<WorkerThreadStrategy>& strategy = i->strategy();
        if(strategy->isIdle())
        {
            strategy->markBusy();
            return i;
        }
    }

    return createWorkerThread();
}

sp<ExecutorWorkerThread> ExecutorThreadPool::createWorkerThread()
{
    const sp<ExecutorWorkerThread> workerThread = sp<ExecutorWorkerThread>::make(sp<WorkerThreadStrategy>::make(_stub), "Worker");
    _stub->_worker_threads.push(workerThread);
    _stub->_worker_count ++;
    return workerThread;
}

ExecutorThreadPool::Stub::Stub(sp<Executor> exceptionExecutor, uint32_t capacity)
    : _exception_executor(std::move(exceptionExecutor)), _capacity(capacity), _worker_count(0)
{
}

ExecutorThreadPool::WorkerThreadStrategy::WorkerThreadStrategy(const sp<ExecutorThreadPool::Stub>& stub)
    : _stub(stub), _idled_cycle(0)
{
}

void ExecutorThreadPool::WorkerThreadStrategy::onStart()
{
}

uint64_t ExecutorThreadPool::WorkerThreadStrategy::onBusy()
{
    _idled_cycle = 0;
    return 0;
}

void ExecutorThreadPool::WorkerThreadStrategy::onExit()
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
    _stub->_worker_count --;
}

void ExecutorThreadPool::WorkerThreadStrategy::onException(const std::exception& e)
{
    if(_stub->_exception_executor)
        _stub->_exception_executor->execute(sp<ThrowException>::make(e.what()));
}

uint64_t ExecutorThreadPool::WorkerThreadStrategy::onIdle(Thread& thread)
{
    _idled_cycle ++;
    if(_idled_cycle > 20000 && _stub->_worker_count.load(std::memory_order_relaxed) > _stub->_capacity)
        thread.terminate();
    return 20000;
}

bool ExecutorThreadPool::WorkerThreadStrategy::isIdle() const
{
    return _idled_cycle > 0;
}

void ExecutorThreadPool::WorkerThreadStrategy::markBusy()
{
    _idled_cycle = 0;
}

}
