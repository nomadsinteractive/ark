#include "test/base/test_case.h"

#include <chrono>
#include <random>
#include <thread>

#include "core/ark.h"
#include "core/base/message_loop.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/concurrent/lf_stack.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/traits/disposed.h"
#include "core/types/shared_ptr.h"

#include "core/base/thread.h"

#include "app/base/application_context.h"
#include "app/base/message_loop_thread.h"

#include "platform/platform.h"



namespace ark {
namespace unittest {

class RunnableImpl : public Runnable {
public:
    RunnableImpl(const String& tag)
        : _tick(0), _tag(tag) {
    }

    virtual void run() {
        printf("%s = %d\n", _tag.c_str(), _tick ++);
    }

    uint32_t tick() const {
        return _tick;
    }

private:
    uint32_t _tick;
    String _tag;
};

class MessageLoopTestCase : public TestCase {
public:
    virtual int launch() override {
        const sp<MessageLoopThread> messageLoopThread = sp<MessageLoopThread>::make(Platform::getSteadyClock());
        sp<RunnableImpl> task = sp<RunnableImpl>::make("a");
        sp<RunnableImpl> task1 = sp<RunnableImpl>::make("b");
        sp<RunnableImpl> task2 = sp<RunnableImpl>::make("c");
        sp<Disposed> expirable = sp<Disposed>::make();

        messageLoopThread->post(task, 0.2f);
        messageLoopThread->schedule(task1, 0.3f);
        messageLoopThread->schedule(task2.absorb(expirable), 0.3f);
        expirable->dispose();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        messageLoopThread->thread().terminate();
        messageLoopThread->thread().join();

        return task->tick() == 1 && task1->tick() == 4 && task2->tick() == 1 ? 0 : -1;
    }
};

}
}

ark::unittest::TestCase* message_loops_create() {
    return new ark::unittest::MessageLoopTestCase();
}
