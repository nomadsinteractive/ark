#ifndef ARK_APP_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_THREAD_H_
#define ARK_APP_IMPL_MESSAGE_LOOP_MESSAGE_LOOP_THREAD_H_

#include "core/base/api.h"
#include "core/base/thread.h"
#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/inf/message_loop.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

class MessageLoopThread : public MessageLoop {
public:
    MessageLoopThread(const sp<MessageLoop>& messageLoop);
    ~MessageLoopThread();

    virtual void post(const sp<Runnable>& task, float delay) override;
    virtual void schedule(const sp<Runnable>& task, float interval) override;

    void start();
    void pause();
    void resume();
    void terminate();
    void join();

    const Thread& thread() const;
    Thread& thread();

private:
    class RunnableImpl : public Runnable {
    public:
        RunnableImpl(const Thread& thread, const sp<MessageLoop>& messageLoop);

        virtual void run() override;

    private:
        Thread _thread;
        sp<MessageLoop> _message_loop;
    };

private:
    Thread _thread;
    sp<MessageLoop> _message_loop;
    sp<RunnableImpl> _runnable_impl;

};

}

#endif
