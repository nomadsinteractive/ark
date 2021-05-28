#include "core/impl/executor/executor_worker_thread.h"

namespace ark {

ExecutorWorkerThread::ExecutorWorkerThread(sp<ExecutorWorkerThread::Strategy> strategy)
    : _worker(sp<Worker>::make(_thread, std::move(strategy)))
{
    _thread.setEntry(_worker);
    _thread.start();
}

void ExecutorWorkerThread::execute(const sp<Runnable>& task)
{
    _worker->_pending_tasks.push(task);
    _thread.notify();
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
    _strategy->onStart();

    uint64_t waitms = 1000;
    while(_thread.status() != Thread::THREAD_STATE_TERMINATED)
    {
        _thread.wait(std::chrono::microseconds(std::max<uint64_t>(1000, waitms)));

        sp<Runnable> task;
        bool hasTask = _pending_tasks.pop(task);

        if(hasTask)
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

            waitms = _strategy->onBusy();
        }
        else
            waitms = _strategy->onIdle(_thread);
    }

    _strategy->onExit();
}

}
