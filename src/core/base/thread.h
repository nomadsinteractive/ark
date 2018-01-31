#ifndef ARK_CORE_BASE_THREAD_H_
#define ARK_CORE_BASE_THREAD_H_

#include <condition_variable>
#include <mutex>
#include <thread>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Thread {
public:
    enum State {
        THREAD_STATE_NONE,
        THREAD_STATE_RUNNING,
        THREAD_STATE_PAUSED,
        THREAD_STATE_TERMINATED
    };

    typedef std::thread::id Id;

    Thread(const sp<Runnable>& entry = nullptr);
    Thread(const Thread& other);

    void resume();
    void pause();
    void terminate();

    void join();
    void detach();

    bool isRunning() const;
    bool isPaused() const;
    bool isTerminated() const;

    void setEntry(const sp<Runnable>& entry);
    void start();

    void wait(uint64_t microseconds = 0);
    void signal();

    template<typename T> static T& local() {
        thread_local T THREAD_LOCAL;
        return THREAD_LOCAL;
    }

    static Id currentThreadId();

public:
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

        State status() const;

        const sp<Runnable>& entry() const;
        void setEntry(const sp<Runnable>& entry);

        bool wait(uint64_t microseconds);
        void notify();

    private:
        State _state;

        sp<Runnable> _entry;

        std::mutex _mutex;
        std::condition_variable _condition_variable;

        std::thread _thread;

    };

    const sp<Stub>& stub() const;

private:
    sp<Stub> _stub;
};

}

#endif
