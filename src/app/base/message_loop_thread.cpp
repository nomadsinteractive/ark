#include "app/base/message_loop_thread.h"

#include <algorithm>

#include "core/util/math.h"

namespace ark {

MessageLoopThread::MessageLoopThread(const sp<MessageLoop>& messageLoop)
    : _message_loop(messageLoop), _runnable_impl(sp<RunnableImpl>::make(_thread, messageLoop))
{
    _thread.setEntry(_runnable_impl);
}

MessageLoopThread::~MessageLoopThread()
{
    _thread.terminate();
}

void MessageLoopThread::post(const sp<Runnable>& task, float delay)
{
    _message_loop->post(task, delay);
    _thread.signal();
}

void MessageLoopThread::schedule(const sp<Runnable>& task, float interval)
{
    _message_loop->schedule(task, interval);
    _thread.signal();
}

void MessageLoopThread::start()
{
    _thread.start();
}

void MessageLoopThread::pause()
{
    _thread.pause();
}

void MessageLoopThread::resume()
{
    _thread.resume();
}

void MessageLoopThread::terminate()
{
    _thread.terminate();
}

void MessageLoopThread::join()
{
    _thread.join();
}

const Thread& MessageLoopThread::thread() const
{
    return _thread;
}

Thread& MessageLoopThread::thread()
{
    return _thread;
}

MessageLoopThread::RunnableImpl::RunnableImpl(const Thread& thread, const sp<MessageLoop>& messageLoop)
    : _thread(thread), _message_loop(messageLoop)
{
}

void MessageLoopThread::RunnableImpl::run()
{
    while(!_thread.isTerminated())
    {
        while(_thread.isPaused())
            _thread.wait(100000);

        uint64_t microsec = _message_loop->pollOnce();
        _thread.wait(std::max<uint64_t>(2000, microsec));
    }
}

}
