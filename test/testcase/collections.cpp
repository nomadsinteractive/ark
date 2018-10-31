#include "core/types/shared_ptr.h"

#include <iostream>

#include <stdint.h>

#include "core/collection/list_with_lifecycle.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/epi/lifecycle.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

template<typename T> class Filter {
public:
    Filter(const sp<T>& v) {
    }

    template<typename U> bool operator()(U& list, typename U::iterator& iterator) const {
        return true;
    }
};

class CollectionsTestCase : public TestCase {
public:
    virtual int launch() override {
        sp<Lifecycle> expirable = sp<Lifecycle>::make();
        ListWithLifecycle<uint32_t> expirableList;

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

        TESTCASE_VALIDATE(sum == 30);

        WeakRefList<uint32_t> s2;
        const sp<uint32_t> i1 = sp<uint32_t>::make(1);
        {
            const sp<uint32_t> i2 = sp<uint32_t>::make(2);
            s2.push_back(i1);
            s2.push_back(i2);
            s2.push_back(sp<uint32_t>::make(4));

            sum = 0;
            for(const sp<uint32_t>& i : s2)
                sum += *i.get();
            TESTCASE_VALIDATE(sum == 3);
        }

        sum = 0;
        for(const sp<uint32_t>& i : s2)
            sum += *i.get();
        TESTCASE_VALIDATE(sum == 1);

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
