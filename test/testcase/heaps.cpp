#include "core/types/shared_ptr.h"

#include <random>

#include "core/base/heap.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class HeapsTestCase : public TestCase {
public:
    virtual int launch() override {
        Heap<uint32_t> heap((1 << 20) * 8);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(4, (1 << 16));

        for(uint32_t i = 0; i < 16; ++i) {
            uint32_t size = dis(gen);
            printf("size required: %d, allocated: %d\n", size, heap.allocate(size));
        }

        return 0;
    }
};

}
}


ark::unittest::TestCase* heaps_create() {
    return new ark::unittest::HeapsTestCase();
}
