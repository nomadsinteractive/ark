#include "app/base/message_loop_thread.h"

#include "core/inf/variable.h"
#include "core/impl/message_loop/message_loop_default.h"

#include "platform/platform.h"

namespace ark {

MessageLoopThread::MessageLoopThread(const sp<Variable<uint64_t>>& ticker)
    : _ticker(ticker), _message_loop(sp<MessageLoopDefault>::make(ticker)), _runnable_impl(sp<RunnableImpl>::make(_thread, _message_loop, ticker))
{
    _thread.setEntry(_runnable_impl);
}

MessageLoopThread::~MessageLoopThread()
{
    _thread.terminate();
}

void MessageLoopThread::post(const sp<Runnable>& task, float delay)
{
    _message_loop->post(task, _ticker->val() + static_cast<uint64_t>(delay * 1000000));
    _thread.signal();
}

void MessageLoopThread::schedule(const sp<Runnable>& task, float interval)
{
    _message_loop->schedule(task, static_cast<uint32_t>(interval * 1000000));
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

//void MessageLoopThread::pollOnce()
//{
//    uint64_t now = _ticker->val();
//    _runnable_impl->_message_loop->pollOnce(now);
//}

const Thread& MessageLoopThread::thread() const
{
    return _thread;
}

MessageLoopThread::RunnableImpl::RunnableImpl(const Thread& thread, const sp<MessageLoopDefault>& messageLoop, const sp<Variable<uint64_t>>& ticker)
    : _thread(thread), _message_loop(messageLoop), _ticker(ticker), _wait_duration(std::chrono::milliseconds(1))
{
}

void MessageLoopThread::RunnableImpl::run()
{
    while(!_thread.isTerminated())
    {
        while(_thread.isPaused())
            _thread.wait(std::chrono::milliseconds(100));

        uint64_t now = _ticker->val();
        uint64_t nextFireTick = _message_loop->pollOnce();
        if(now < nextFireTick)
            _thread.wait(_wait_duration, WaitPredicate(_ticker, nextFireTick));
    }
}

MessageLoopThread::WaitPredicate::WaitPredicate(Variable<uint64_t>& ticker, uint64_t until)
    : _ticker(ticker), _until(until)
{
}

bool MessageLoopThread::WaitPredicate::operator()()
{
    return _until <= _ticker.val();
}

}
