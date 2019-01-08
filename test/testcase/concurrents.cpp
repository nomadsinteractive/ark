#include "core/types/shared_ptr.h"

#include <list>
#include <queue>

#include "core/concurrent/lock_free_stack.h"
#include "core/concurrent/one_consumer_synchronized.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class ConcurrentsTestCase : public TestCase {
public:
    virtual int launch() override {
        {
            LockFreeStack<int32_t> s2;
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
        return 0;
    }
};

}
}


ark::unittest::TestCase* concurrents_create() {
    return new ark::unittest::ConcurrentsTestCase();
}
