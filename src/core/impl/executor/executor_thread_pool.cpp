#include "core/impl/executor/executor_thread_pool.h"

#include "core/base/string.h"
#include "core/inf/message_loop.h"

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

ExecutorThreadPool::ExecutorThreadPool(const sp<MessageLoop>& messageLoop, uint32_t capacity)
    : _stub(sp<Stub>::make(messageLoop, std::max<uint32_t>(2, capacity ? capacity : std::thread::hardware_concurrency())))
{
}

void ExecutorThreadPool::execute(const sp<Runnable>& task)
{
//    for(const sp<Worker>& worker : _stub->_workers)
//        if(worker->idle())
//            return worker->post(task);

//    createWorker()->post(task);
    for(const sp<ExecutorWorkerThread>& i : _stub->_worker_threads)
    {
        const sp<WorkerThreadStrategy> strategy = i->strategy();
        if(strategy->idle())
            return i->execute(task);
    }

    createWorkerThread()->execute(task);

}

//sp<ExecutorThreadPool::Worker> ExecutorThreadPool::createWorker()
//{
//    Thread thread;
//    const sp<Worker> worker = sp<Worker>::make(_stub, thread);
//    _stub->_workers.push(worker);
//    _stub->_worker_count ++;
//    thread.setEntry(worker);
//    thread.start();
//    return worker;
//}

sp<ExecutorWorkerThread> ExecutorThreadPool::createWorkerThread()
{
    const sp<ExecutorWorkerThread> workerThread = sp<ExecutorWorkerThread>::make(sp<WorkerThreadStrategy>::make(_stub));
    _stub->_worker_threads.push(workerThread);
    _stub->_worker_count ++;
    return workerThread;
}

//ExecutorThreadPool::Worker::Worker(const sp<Stub>& stub, const Thread& thread)
//    : _stub(stub), _thread(thread), _idle(false), _idled_cycle(0)
//{
//}

//ExecutorThreadPool::Worker::~Worker()
//{
//    _thread.detach();
//    _thread.terminate();
//}

//bool ExecutorThreadPool::Worker::idle() const
//{
//    return _idle;
//}

//void ExecutorThreadPool::Worker::run()
//{
//    const auto duration = std::chrono::milliseconds(1);
//    while(_thread.status() != Thread::THREAD_STATE_TERMINATED)
//    {
//        _thread.wait(duration);

//        sp<Runnable> task;
//        bool notEmpty = _pendings.pop(task);

//        if(notEmpty)
//        {
//            _idle = false;
//            _idled_cycle = 0;

//            do
//            {
//                try {
//                    task->run();
//                }
//                catch(const std::exception& e) {
//                    if(_stub->_message_loop)
//                        _stub->_message_loop->post(sp<ThrowException>::make(e.what()), 0);
//                }
//            } while(_pendings.pop(task));
//        }
//        else
//        {
//            _idle = true;
//            _idled_cycle ++;
//            if(_idled_cycle > 20000 && _stub->_worker_count.load(std::memory_order_relaxed) > _stub->_capacity)
//                break;
//        }
//    }
//    removeSelf();
//    DCHECK(_stub->_worker_count > 0, "Worker count mismatch");
//    _stub->_worker_count --;
//}

//void ExecutorThreadPool::Worker::removeSelf()
//{
//    bool removed = false;
//    const std::lock_guard<std::mutex> guard(_stub->_mutex);
//    for(const sp<Worker>& worker : _stub->_workers.clear())
//        if(worker.get() != this)
//            _stub->_workers.push(worker);
//        else
//            removed = true;
//    DCHECK(removed, "Unable to remove Worker: %p", this);
//}

//void ExecutorThreadPool::Worker::post(sp<Runnable> task)
//{
//    _idle = false;
//    _pendings.push(std::move(task));
//    _thread.notify();
//}

ExecutorThreadPool::Stub::Stub(const sp<MessageLoop>& messageLoop, uint32_t capacity)
    : _message_loop(messageLoop), _capacity(capacity), _worker_count(0)
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
    if(_stub->_message_loop)
        _stub->_message_loop->post(sp<ThrowException>::make(e.what()), 0);
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
