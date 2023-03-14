#include "core/types/shared_ptr.h"

#include <list>
#include <queue>
#include <random>
#include <thread>

#include "core/ark.h"
#include "core/base/string.h"
#include "core/concurrent/lf_queue.h"
#include "core/concurrent/lf_stack.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/impl/executor/executor_thread_pool.h"

#include "app/base/application_context.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

template<typename T> class Sleeper : public Runnable {
public:
    Sleeper(const String& tag, int32_t level, const sp<T>& container, bool& flag)
        : _tag(tag), _level(level), _container(container), _flag(flag) {
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
                _container->push(data);
            else
                _container->pop(data);
        }
        printf("[%s] done after %lld milliseconds\n", _tag.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s).count());
        _flag = true;
    }

private:
    String _tag;
    int32_t _level;
    sp<T> _container;

    bool& _flag;
};

class ConcurrentsTestCase : public TestCase {
public:
    virtual int launch() override {
        {
            LFStack<int32_t> s2;
            std::list<int32_t> l1 = {1, 2, 3};
            s2.assign(l1);

            int32_t r = 0;
            if(!s2.pop(r) && r != 1)
                return 1;
            if(!s2.pop(r) && r != 2)
                return 2;
            if(!s2.pop(r) && r != 3)
                return 3;
        }
        {
            OCSQueue<size_t> queue;
            queue.add(0);
            queue.add(1);
            queue.add(2);

            size_t size = queue.size();
            for(size_t i = 0; i < size; ++i)
            {
                size_t data;
                TESTCASE_VALIDATE(queue.pop(data));
                TESTCASE_VALIDATE(data == i);
            }
        }
        {
            const sp<Executor> executor = Ark::instance().applicationContext()->executorThreadPool();
            const sp<LFStack<int32_t>> stack = sp<LFStack<int32_t>>::make();

            bool flag1 = true, flag2 = true, flag3 = true;
            executor->execute(sp<Sleeper<LFStack<int32_t>>>::make("LFStack_01", 20, stack, flag1));
            executor->execute(sp<Sleeper<LFStack<int32_t>>>::make("LFStack_02", 50, stack, flag2));
            executor->execute(sp<Sleeper<LFStack<int32_t>>>::make("LFStack_03", 100, stack, flag3));
            while(!flag1 || !flag2 || !flag3)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        {
            const sp<Executor>& executor = Ark::instance().applicationContext()->executorThreadPool();
            const sp<LFQueue<int32_t>> queue = sp<LFQueue<int32_t>>::make();

            bool flag1 = true, flag2 = true, flag3 = true;
            executor->execute(sp<Sleeper<LFQueue<int32_t>>>::make("LFQueue_01", 20, queue, flag1));
            executor->execute(sp<Sleeper<LFQueue<int32_t>>>::make("LFQueue_02", 50, queue, flag2));
            executor->execute(sp<Sleeper<LFQueue<int32_t>>>::make("LFQueue_03", 100, queue, flag3));
            while(!flag1 || !flag2 || !flag3)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return 0;
    }
};

}
}


ark::unittest::TestCase* concurrents_create() {
    return new ark::unittest::ConcurrentsTestCase();
}
