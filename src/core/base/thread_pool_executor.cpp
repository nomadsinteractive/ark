#include "core/base/thread_pool_executor.h"

namespace ark {

ThreadPoolExecutor::ThreadPoolExecutor(uint32_t capacity)
    : _capacity(std::max<uint32_t>(2, capacity ? capacity : std::thread::hardware_concurrency()))
{
}

void ThreadPoolExecutor::execute(const sp<Runnable>& task)
{
    uint32_t count = 0;
    for(const sp<Worker>& worker : _workers)
    {
        if(!worker->busy())
        {
            worker->post(task);
            return;
        }
        count ++;
    }
    CHECK(count < _capacity, "All threads are busy, and cannot create more(capacity = %d)", _capacity);
    createWorker()->post(task);
}

sp<ThreadPoolExecutor::Worker> ThreadPoolExecutor::createWorker()
{
    Thread thread;
    const sp<Worker> worker = sp<Worker>::make(thread.stub());
    _workers.push(worker);
    thread.setEntry(worker->entry());
    thread.start();
    return worker;
}

ThreadPoolExecutor::Worker::Worker(const sp<Thread::Stub>& stub)
    : _thread_stub(stub), _stub(sp<Stub>::make(stub))
{
}

ThreadPoolExecutor::Worker::~Worker()
{
    _thread_stub->detach();
    _thread_stub->terminate();
}

bool ThreadPoolExecutor::Worker::busy() const
{
    return _stub->busy();
}

void ThreadPoolExecutor::Worker::post(const sp<Runnable>& task) const
{
    _stub->post(task);
}

sp<Runnable> ThreadPoolExecutor::Worker::entry() const
{
    return _stub;
}

ThreadPoolExecutor::Worker::Stub::Stub(const sp<Thread::Stub>& threadStub)
    : _thread_stub(threadStub)
{
}

void ThreadPoolExecutor::Worker::Stub::run()
{
    while(_thread_stub->status() != Thread::THREAD_STATE_TERMINATED)
    {
        _busy = false;
        _thread_stub->wait(100000);
        sp<Runnable> front;
        if(_pendings.pop(front))
        {
            _busy = true;
            front->run();
        }
    }
}

bool ThreadPoolExecutor::Worker::Stub::busy() const
{
    return _busy;
}

void ThreadPoolExecutor::Worker::Stub::post(const sp<Runnable>& task)
{
    _pendings.push(task);
    _thread_stub->notify();
}

}
