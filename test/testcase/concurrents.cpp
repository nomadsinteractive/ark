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
            OCSQueue<uint32_t> queue;
            queue.push(0);
            queue.push(1);
            queue.push(2);

            std::queue<uint32_t> cleared = queue.clear();
            uint32_t pos = 0;
            while(!cleared.empty())
            {
                if(cleared.front() != (pos++))
                    return 2 + pos;
                cleared.pop();
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
