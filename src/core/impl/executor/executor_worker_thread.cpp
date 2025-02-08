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

void ExecutorWorkerThread::execute(sp<Runnable> task)
{
    _worker->_pending_tasks.push(std::move(task));
    _thread.notify();
}

void ExecutorWorkerThread::terminate() const
{
    if(_thread.isTerminated())
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
    _strategy->onStart();

    while(_thread.status() != Thread::THREAD_STATE_TERMINATED)
    {
        _thread.wait(std::chrono::milliseconds(1));

        sp<Runnable> task;
        if(_pending_tasks.pop(task))
        {
            do
            {
                try {
                    task->run();
                }
                catch(const std::exception& e) {
                    _strategy->onException(e);
                }
            } while(_pending_tasks.pop(task));

            _strategy->onBusy();
        }
        else
            _strategy->onIdle(_thread);
    }

    _strategy->onExit();
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
