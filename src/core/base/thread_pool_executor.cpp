#include "core/base/thread_pool_executor.h"

#include "core/inf/message_loop.h"

namespace ark {

ThreadPoolExecutor::ThreadPoolExecutor(const sp<MessageLoop>& messageLoop, uint32_t capacity)
    : _stub(sp<Stub>::make(messageLoop, std::max<uint32_t>(2, capacity ? capacity : std::thread::hardware_concurrency())))
{
}

void ThreadPoolExecutor::execute(const sp<Runnable>& task)
{
    for(const sp<Worker>& worker : _stub->_workers)
        if(worker->idle())
        {
            worker->post(task);
            return;
        }
    createWorker()->post(task);
}

sp<ThreadPoolExecutor::Worker> ThreadPoolExecutor::createWorker()
{
    Thread thread;
    const sp<Worker> worker = sp<Worker>::make(_stub, thread.stub());
    _stub->_workers.push(worker);
    _stub->_worker_count ++;
    thread.setEntry(worker);
    thread.start();
    return worker;
}

ThreadPoolExecutor::Worker::Worker(const sp<Stub>& stub, const sp<Thread::Stub>& threadStub)
    : _stub(stub), _thread_stub(threadStub), _idle(false), _idled_cycle(0)
{
}

ThreadPoolExecutor::Worker::~Worker()
{
    _thread_stub->detach();
    _thread_stub->terminate();
}

bool ThreadPoolExecutor::Worker::idle() const
{
    return _idle;
}

void ThreadPoolExecutor::Worker::run()
{
    const auto duration = std::chrono::milliseconds(1);
    while(_thread_stub->status() != Thread::THREAD_STATE_TERMINATED)
    {
        _thread_stub->wait(duration);
        sp<Runnable> front;
        if(_pendings.pop(front))
        {
            _idle = false;
            _idled_cycle = 0;
            try {
                front->run();
            }
            catch(const std::exception& e) {
                if(_stub->_message_loop)
                    _stub->_message_loop->postTask([e]() {
                        FATAL("Unhanlded exception in thread: %s", e.what());
                    });
            }
        }
        else
        {
            _idle = true;
            _idled_cycle ++;
            if(_idled_cycle > 20000 && _stub->_worker_count.load(std::memory_order_relaxed) > _stub->_capacity)
                break;
        }
    }
    removeSelf();
    DCHECK(_stub->_worker_count > 0, "Worker count mismatch");
    _stub->_worker_count --;
}

void ThreadPoolExecutor::Worker::removeSelf()
{
    bool removed = false;
    const std::lock_guard<std::mutex> guard(_stub->_mutex);
    for(const sp<Worker>& worker : _stub->_workers.clear())
        if(worker.get() != this)
            _stub->_workers.push(worker);
        else
            removed = true;
    DCHECK(removed, "Unable to remove Worker: %p", this);
}

void ThreadPoolExecutor::Worker::post(const sp<Runnable>& task)
{
    _idle = false;
    _pendings.add(task);
    _thread_stub->notify();
}

ThreadPoolExecutor::Stub::Stub(const sp<MessageLoop>& messageLoop, uint32_t capacity)
    : _message_loop(messageLoop), _capacity(capacity), _worker_count(0)
{
}

}
