#ifndef ARK_APP_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_THREAD_H_
#define ARK_APP_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_THREAD_H_

#include <chrono>

#include "core/base/message_loop.h"
#include "core/base/thread.h"
#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

class MessageLoopThread {
public:
    MessageLoopThread(const sp<Variable<uint64_t>>& ticker);
    ~MessageLoopThread();

    void post(const sp<Runnable>& task, float delay);
    void schedule(const sp<Runnable>& task, float interval);

    const Thread& thread() const;

private:
    class WaitPredicate {
    public:
        WaitPredicate(Variable<uint64_t>& ticker, uint64_t until);

        bool operator()();

    private:
        Variable<uint64_t>& _ticker;
        uint64_t _until;

    };

    class RunnableImpl : public Runnable {
    public:
        RunnableImpl(const Thread& thread, const sp<MessageLoop>& messageLoop, const sp<Variable<uint64_t>>& ticker);

        virtual void run() override;

    private:
        Thread _thread;
        sp<MessageLoop> _message_loop;
        sp<Variable<uint64_t>> _ticker;

        std::chrono::duration<long long, std::micro> _wait_duration;
    };

private:
    Thread _thread;
    sp<MessageLoop> _message_loop;
    sp<RunnableImpl> _runnable_impl;

};

}

#endif
