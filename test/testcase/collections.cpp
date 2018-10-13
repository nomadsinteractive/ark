#include "core/types/shared_ptr.h"

#include <iostream>

#include <stdint.h>

#include "core/collection/expirable_item_list.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/epi/lifecycle.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class CollectionsTestCase : public TestCase {
public:
    virtual int launch() override {
        sp<Lifecycle> expirable = sp<Lifecycle>::make();
        ExpirableItemList<uint32_t> expirableList;

        for(uint32_t i = 0; i < 10; i++)
        {
            sp<uint32_t> ptr = sp<uint32_t>::adopt((new uint32_t(i)));
            if(i % 4 == 1)
                ptr.absorb<Lifecycle>(expirable);
            expirableList.push_back(ptr);
        }
        expirable->dispose();

        uint32_t sum = 0;
        for(const sp<uint32_t>& i : expirableList)
            sum += *i.get();
        if(sum != 30)
            return 1;

        LockFreeStack<uint32_t> slist;
        slist.push(10);
        slist.push(20);

        for(uint32_t i : slist.clear())
            std::cout << i << std::endl;

        return 0;
    }
};

}
}


ark::unittest::TestCase* collections_create() {
    return new ark::unittest::CollectionsTestCase();
}
