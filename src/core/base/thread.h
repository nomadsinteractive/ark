#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "core/base/api.h"
#include "core/base/mutex.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Thread {
private:
    class Stub;

public:
    enum State {
        THREAD_STATE_NONE,
        THREAD_STATE_RUNNING,
        THREAD_STATE_PAUSED,
        THREAD_STATE_TERMINATED
    };

    typedef std::size_t Id;

    Thread(const sp<Runnable>& entry = nullptr);
    Thread(const sp<Stub>& stub);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Thread);

    void resume() const;
    void pause() const;
    void terminate() const;

    void join() const;
    void detach() const;

    bool isRunning() const;
    bool isPaused() const;
    bool isTerminated() const;

    const sp<Runnable>& entry() const;
    void setEntry(const sp<Runnable>& entry) const;

    void start();

    template<typename T> bool wait(const T& duration) const {
        return _stub->_mutex.wait<T>(duration);
    }
    template<typename T, typename U> bool wait(const T& duration, U pred) const {
        return _stub->_mutex.wait<T, U>(duration, std::move(pred));
    }

    State status() const;
    void notify() const;

    template<typename T> static T& local() {
        thread_local T THREAD_LOCAL;
        return THREAD_LOCAL;
    }

    static Id currentThreadId();

private:
    class Stub {
    public:
        Stub(const sp<Runnable>& entry);
        ~Stub();

        void start(const sp<Stub>& self);
        void resume();
        void pause();
        void terminate();
        void join();
        void detach();

    private:
        State _state;

        sp<Runnable> _entry;
        Mutex _mutex;
        std::thread _thread;

        friend class Thread;
    };

private:
    sp<Stub> _stub;
};

}
