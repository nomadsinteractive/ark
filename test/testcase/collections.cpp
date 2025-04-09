#include "core/types/shared_ptr.h"

#include <iostream>

#include <stdint.h>

#include "core/collection/list.h"
#include "core/collection/bitwise_trie.h"
#include "core/concurrent/lf_stack.h"
#include "core/components/discarded.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class CollectionsTestCase : public TestCase {
public:
    virtual int launch() override {
        sp<Discarded> expirable = sp<Discarded>::make();
        D_FList<uint32_t> expirableList;

        for(uint32_t i = 0; i < 10; i++)
            expirableList.emplace_back(i, i % 4 == 1 ? expirable : nullptr);
        expirable->discard();

        uint32_t sum = 0;
        for(uint32_t i : expirableList.update(0))
            sum += i;

        TESTCASE_VALIDATE(sum == 30);

        LFStack<uint32_t> slist;
        slist.push(10);
        slist.push(20);

        for(uint32_t i : slist.clear())
            std::cout << i << std::endl;


        {
            BitwiseTrie<uint32_t, uint32_t> trie;

            for(uint32_t i = 0; i < 32; ++i)
                trie.put(i, i);

            for(uint32_t i = 8; i < 16; ++i)
                trie.put(1 << i, 1 << i);

            trie.remove(0);
            trie.remove(256);

            for(uint32_t i = 0; i < 16; ++i)
            {
                uint32_t key = (1 << i) - 1;
                uint32_t* val = trie.find(key);
                if(key == 0)
                    TESTCASE_VALIDATE(*val == 1);
                else if(key < 32)
                    TESTCASE_VALIDATE(*val == key);
                else if(key < 512)
                    TESTCASE_VALIDATE(*val == 512);
                else
                    TESTCASE_VALIDATE(*val == key + 1);
            }
        }

        return 0;
    }
};

}
}


ark::unittest::TestCase* collections_create() {
    return new ark::unittest::CollectionsTestCase();
}
