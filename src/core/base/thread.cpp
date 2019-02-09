#include "core/base/thread.h"

#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "platform/platform.h"

namespace ark {

static void _thread_main_entry(const sp<Thread::Stub> stub)
{
    const sp<Runnable> entry = stub->entry();
    stub->resume();
    stub->setEntry(nullptr);
    entry->run();
    Platform::detachCurrentThread();
}

Thread::Thread(const sp<Runnable>& entry)
    : _stub(sp<Stub>::make(entry))
{
}

Thread::Thread(const Thread& other)
    : _stub(other._stub)
{
}

void Thread::setEntry(const sp<Runnable>& entry)
{
    _stub->setEntry(entry);
}

void Thread::start()
{
    _stub->start(_stub);
}

void Thread::signal() const
{
    _stub->notify();
}

void Thread::resume() const
{
    _stub->resume();
}

void Thread::pause() const
{
    _stub->pause();
}

void Thread::terminate() const
{
    _stub->terminate();
}

void Thread::join() const
{
    _stub->join();
}

void Thread::detach() const
{
    _stub->detach();
}

bool Thread::isRunning() const
{
    return _stub->status() == THREAD_STATE_RUNNING;
}

bool Thread::isPaused() const
{
    return _stub->status() == THREAD_STATE_PAUSED;
}

bool Thread::isTerminated() const
{
    return _stub->status() == THREAD_STATE_TERMINATED;
}

Thread::Id Thread::currentThreadId()
{
    return std::this_thread::get_id();
}

const sp<Thread::Stub>& Thread::stub() const
{
    return _stub;
}

Thread::Stub::Stub(const sp<Runnable>& entry)
    : _state(THREAD_STATE_NONE), _entry(entry)
{
}

Thread::Stub::~Stub()
{
    detach();
}

void Thread::Stub::start(const sp<Stub>& self)
{
    DCHECK(_state == THREAD_STATE_NONE && _entry, "Illegal state(%d). Could not start thread", _state);
    LOGD("");
    _thread = std::thread(_thread_main_entry, self);
}

void Thread::Stub::resume()
{
    DCHECK(_state == THREAD_STATE_PAUSED || _state == THREAD_STATE_NONE, "Illegal state(%d). Could not resume thread", _state);
    LOGD("");
    _state = THREAD_STATE_RUNNING;
}

void Thread::Stub::pause()
{
    DCHECK(_state == THREAD_STATE_RUNNING, "Illegal state(%d). Could not pause thread", _state);
    LOGD("");
    _state = THREAD_STATE_PAUSED;
}

void Thread::Stub::terminate()
{
    LOGD("");
    _state = THREAD_STATE_TERMINATED;
}

void Thread::Stub::join()
{
    DCHECK(_thread.joinable(), "Illegal state. Could not join thread");
    LOGD("");
    _thread.join();
}

void Thread::Stub::detach()
{
    LOGD("");
    if(_thread.joinable())
        _thread.detach();
}

Thread::State Thread::Stub::status() const
{
    return _state;
}

const sp<Runnable>& Thread::Stub::entry() const
{
    return _entry;
}

void Thread::Stub::setEntry(const sp<Runnable>& entry)
{
    if(!entry || _state == THREAD_STATE_NONE)
        _entry = entry;
    else
        FATAL("Cannot set entry after thread being started");
}

void Thread::Stub::notify()
{
    _condition_variable.notify_one();
}

}
