#include "core/impl/executor/executor_thread_pool.h"

#include "core/base/string.h"

namespace ark {

namespace {

class ThrowException : public Runnable {
public:
    ThrowException(const String& what)
        : _what(what) {
    }

    virtual void run() override {
        FATAL("Unhanlded exception in thread: %s", _what.c_str());
    }

private:
    String _what;
};

}

ExecutorThreadPool::ExecutorThreadPool(const sp<Executor>& executor, uint32_t capacity)
    : _stub(sp<Stub>::make(executor, std::max<uint32_t>(2, capacity ? capacity : std::thread::hardware_concurrency())))
{
}

void ExecutorThreadPool::execute(const sp<Runnable>& task)
{
    for(const sp<ExecutorWorkerThread>& i : _stub->_worker_threads)
    {
        const sp<WorkerThreadStrategy> strategy = i->strategy();
        if(strategy->idle())
            return i->execute(task);
    }

    createWorkerThread()->execute(task);

}

sp<ExecutorWorkerThread> ExecutorThreadPool::createWorkerThread()
{
    const sp<ExecutorWorkerThread> workerThread = sp<ExecutorWorkerThread>::make(sp<WorkerThreadStrategy>::make(_stub));
    _stub->_worker_threads.push(workerThread);
    _stub->_worker_count ++;
    return workerThread;
}

ExecutorThreadPool::Stub::Stub(const sp<Executor>& executor, uint32_t capacity)
    : _executor(executor), _capacity(capacity), _worker_count(0)
{
}

ExecutorThreadPool::WorkerThreadStrategy::WorkerThreadStrategy(const sp<ExecutorThreadPool::Stub>& stub)
    : _stub(stub), _idle(false), _idled_cycle(0)
{
}

void ExecutorThreadPool::WorkerThreadStrategy::onStart()
{
    _idle = true;
}

uint64_t ExecutorThreadPool::WorkerThreadStrategy::onBusy()
{
    _idle = false;
    _idled_cycle = 0;
    return 0;
}

void ExecutorThreadPool::WorkerThreadStrategy::onExit()
{
    bool removed = false;
    const std::lock_guard<std::mutex> guard(_stub->_mutex);
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
    if(_stub->_executor)
        _stub->_executor->execute(sp<ThrowException>::make(e.what()));
}

uint64_t ExecutorThreadPool::WorkerThreadStrategy::onIdle(Thread& thread)
{
    _idle = true;
    _idled_cycle ++;
    if(_idled_cycle > 20000 && _stub->_worker_count.load(std::memory_order_relaxed) > _stub->_capacity)
        thread.terminate();
    return 20000;
}

bool ExecutorThreadPool::WorkerThreadStrategy::idle() const
{
    return _idle;
}

}
