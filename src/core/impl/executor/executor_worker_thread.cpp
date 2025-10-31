#include "core/impl/executor/executor_worker_thread.h"

#include "core/ark.h"

namespace ark {

ExecutorWorkerThread::ExecutorWorkerThread(sp<ExecutorWorkerThread::Strategy> strategy, String name)
    : _worker(sp<Worker>::make(_thread, std::move(strategy)))
{
    _thread.setEntry(_worker);
    _thread.start();
}

ExecutorWorkerThread::~ExecutorWorkerThread()
{
    terminate();
}

void ExecutorWorkerThread::execute(const sp<Runnable>& task)
{
    _worker->_pending_tasks.push(task);
    _thread.notify();
}

void ExecutorWorkerThread::terminate() const
{
    if(!_thread.isTerminated())
    {
        _thread.terminate();
        _thread.notify();
    }
}

void ExecutorWorkerThread::tryJoin() const
{
    _thread.tryJoin();
}

const sp<ExecutorWorkerThread::Strategy>& ExecutorWorkerThread::strategy() const
{
    return _worker->_strategy;
}

ExecutorWorkerThread::Worker::Worker(Thread thread, sp<Strategy> strategy)
    : _thread(std::move(thread)), _strategy(std::move(strategy))
{
}

void ExecutorWorkerThread::Worker::run()
{
    DPROFILER_TRACE("Worker", ApplicationProfiler::CATEGORY_START_THREAD);
    const sp<Strategy> strategy = _strategy;
    strategy->onStart();
    while(_thread.status() != Thread::THREAD_STATE_TERMINATED)
    {
        _thread.wait(std::chrono::milliseconds(1));

        if(Optional<sp<Runnable>> optTask = _pending_tasks.pop())
        {
            do
            {
                try {
                    optTask.value()->run();
                }
                catch(const std::exception& e) {
                    strategy->onException(e);
                }
                optTask = _pending_tasks.pop();
            } while(optTask);

            strategy->onBusy();
        }
        else
            strategy->onIdle(_thread);
    }
    strategy->onExit();
}

ExecutorWorkerThread::WaitPredicate::WaitPredicate(const TimePoint& until)
    : _until(until)
{
}

bool ExecutorWorkerThread::WaitPredicate::operator()() const
{
    return std::chrono::steady_clock::now() >= _until;
}

}
