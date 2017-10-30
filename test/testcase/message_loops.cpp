#include "test/base/test_case.h"

#include <chrono>
#include <random>
#include <thread>

#include "core/ark.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/epi/expired.h"
#include "core/impl/message_loop/message_loop_default.h"
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

class Sleeper : public Runnable {
public:
    Sleeper(const String& tag, int32_t level, const sp<LockFreeStack<int32_t>>& stack, bool& flag)
        : _tag(tag), _level(level), _stack(stack), _flag(flag) {
        _flag = false;
    }

    virtual void run() override {
        auto s = std::chrono::steady_clock::now();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis2(0, _level);
        for(int32_t i = 0; i < 10000000; i ++)
        {
            int32_t data = 902;
            if(dis2(gen) > (i % _level))
                _stack->push(data);
            else
                _stack->pop(data);
        }
        printf("[%s] done after %lld milliseconds\n", _tag.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s).count());
        _flag = true;
    }

private:
    String _tag;
    int32_t _level;
    sp<LockFreeStack<int32_t>> _stack;

    bool& _flag;
};

class MessageLoopTestCase : public TestCase {
public:
    virtual int launch() override {
        sp<MessageLoopThread> messageLoopThread = sp<MessageLoopThread>::make(sp<MessageLoopDefault>::make(Platform::getSteadyClock()));
        messageLoopThread->start();
        sp<RunnableImpl> task = sp<RunnableImpl>::make("a");
        sp<RunnableImpl> task1 = sp<RunnableImpl>::make("b");
        sp<RunnableImpl> task2 = sp<RunnableImpl>::make("c");
        sp<Expired> expirable = sp<Expired>::make();

        bool flag1 = true, flag2 = true, flag3 = true;
        messageLoopThread->post(task, 0.2f);
        messageLoopThread->schedule(task1, 0.3f);
        messageLoopThread->schedule(task2.absorb(expirable), 0.3f);
        expirable->expire();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        messageLoopThread->terminate();
        messageLoopThread->thread().join();

        const sp<Executor>& executor = Ark::instance().applicationContext()->executor();
        const sp<LockFreeStack<int32_t>> s1 = sp<LockFreeStack<int32_t>>::make();

        executor->execute(sp<Sleeper>::make("Alpha", 20, s1, flag1));
        executor->execute(sp<Sleeper>::make("Beta", 50, s1, flag2));
        executor->execute(sp<Sleeper>::make("Gamma", 100, s1, flag3));
        while(!flag1 || !flag2 || !flag3)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return task->tick() == 1 && task1->tick() == 4 && task2->tick() == 1 ? 0 : -1;
    }
};

}
}

ark::unittest::TestCase* message_loops_create() {
    return new ark::unittest::MessageLoopTestCase();
}
