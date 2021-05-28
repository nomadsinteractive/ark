#include "app/base/message_loop_thread.h"

#include "core/inf/variable.h"
#include "core/impl/message_loop/message_loop_default.h"

#include "platform/platform.h"

namespace ark {

MessageLoopThread::MessageLoopThread(const sp<Variable<uint64_t>>& ticker)
    : _message_loop(sp<MessageLoopDefault>::make(ticker)), _runnable_impl(sp<RunnableImpl>::make(_thread, _message_loop, ticker))
{
    _thread.setEntry(_runnable_impl);
    _thread.start();
}

MessageLoopThread::~MessageLoopThread()
{
    _thread.terminate();
}

void MessageLoopThread::post(const sp<Runnable>& task, float delay)
{
    _message_loop->post(task, delay);
    _thread.notify();
}

void MessageLoopThread::schedule(const sp<Runnable>& task, float interval)
{
    _message_loop->schedule(task, interval);
    _thread.notify();
}

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

        uint64_t nextFireTick = _message_loop->pollOnce();
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
