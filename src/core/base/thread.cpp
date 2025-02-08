#include "core/base/thread.h"

#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "platform/platform.h"

namespace ark {

static void _thread_main_entry(const Thread thread)
{
    const sp<Runnable> entry = thread.entry();
    thread.resume();
    thread.setEntry(nullptr);
    entry->run();
    Platform::detachCurrentThread();
}

Thread::Thread(const sp<Runnable>& entry)
    : _stub(sp<Stub>::make(entry))
{
}

Thread::Thread(const sp<Thread::Stub>& stub)
    : _stub(stub)
{
}

const sp<Runnable>& Thread::entry() const
{
    return _stub->_entry;
}

void Thread::setEntry(const sp<Runnable>& entry) const
{
    DCHECK(!entry || _stub->_state == THREAD_STATE_NONE, "Cannot set entry after thread being started");
    _stub->_entry = entry;
}

void Thread::start()
{
    _stub->start(_stub);
}

Thread::State Thread::status() const
{
    return _stub->_state;
}

void Thread::notify() const
{
    _stub->_mutex.notify();
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

void Thread::tryJoin() const
{
    if(_stub->_thread.joinable())
        _stub->join();
}

void Thread::detach() const
{
    _stub->detach();
}

bool Thread::isRunning() const
{
    return _stub->_state == THREAD_STATE_RUNNING;
}

bool Thread::isPaused() const
{
    return _stub->_state == THREAD_STATE_PAUSED;
}

bool Thread::isTerminated() const
{
    return _stub->_state == THREAD_STATE_TERMINATED;
}

Thread::Id Thread::currentThreadId()
{
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
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
    _thread = std::thread(_thread_main_entry, Thread(self));
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

}
