#include "core/types/shared_ptr.h"

#include <random>
#include <vector>

#include "core/base/heap.h"
#include "core/inf/array.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class HeapsTestCase : public TestCase {
public:
    enum {
        HEAP_SIZE = (1 << 20)
    };

    struct Memory {
        Memory(size_t size)
            : _memory(sp<ByteArray::Allocated>::make(size)) {
            memset(_memory->buf(), 0, size);
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Memory);

        uint8_t* begin() const {
            return _memory->buf();
        }

        uint8_t* end() const {
            return _memory->buf() + _memory->length();
        }

        sp<ByteArray::Allocated> _memory;
    };

    typedef Heap<Memory, uint32_t> MemoryHeap;

    struct Allocated {
        Allocated(uint8_t* ptr, uint32_t size, uint32_t ttl)
            : _ptr(ptr), _size(size), _ttl(ttl) {
        }

        bool aquire() {
            for(uint32_t i = 0; i < _size; ++i)
                if(_ptr[i] != 0)
                    return false;

            memset(_ptr, 0xfe, _size);
            return true;
        }

        void free(MemoryHeap& heap) {
            memset(_ptr, 0, _size);
            heap.free(_ptr);
        }

        uint8_t* _ptr;
        uint32_t _size;
        uint32_t _ttl;
    };

    Allocated allocate(MemoryHeap& heap, uint32_t size, uint32_t ttl) {
        Optional<uint8_t*> ptr = heap.allocate(size);
        CHECK(ptr, "Unable to allocate block, size: %d", size);
        return Allocated(ptr.value(), size, ttl);
    }

    virtual int launch() override {
        {
            MemoryHeap heap(Memory(1 << 13), sp<MemoryHeap::L1>::make(64));
            heap.extend(Memory(HEAP_SIZE), sp<MemoryHeap::L2>::make());

            TESTCASE_VALIDATE(cycleTest(heap, 128) == 0);
        }

        {
            MemoryHeap heap(Memory(HEAP_SIZE), sp<MemoryHeap::L2>::make());

            TESTCASE_VALIDATE(cycleTest(heap, 128) == 0);
        }

        return 0;
    }

    int32_t cycleTest(MemoryHeap& heap, uint32_t cycleCount) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis1(4, HEAP_SIZE >> 6);
        std::uniform_int_distribution<uint32_t> dis2(4, 512);

        std::vector<Allocated> allocations;

        for(uint32_t i = 0; i < cycleCount; ++i) {
            printf("Cycle %d: allocated: %d available: %d\n", i, heap.allocated(), heap.available());

            std::uniform_int_distribution<uint32_t> ttldis(1, cycleCount - i);

            Allocated a1 = allocate(heap, dis1(gen), ttldis(gen));
            TESTCASE_VALIDATE(a1.aquire());
            allocations.push_back(a1);

            for(uint32_t j = 0; j < 32; ++j) {
                Allocated a2 = allocate(heap, dis2(gen), ttldis(gen));
                TESTCASE_VALIDATE(a2.aquire());
                allocations.push_back(a2);
            }

            for(auto iter = allocations.begin(); iter != allocations.end(); ++iter) {
                Allocated& i = *iter;
                --i._ttl;
                if(i._ttl == 0) {
                    i.free(heap);
                    iter = allocations.erase(iter);
                    if(iter == allocations.end())
                        break;
                }
            }
        }

        return 0;
    }
};

}
}


ark::unittest::TestCase* heaps_create() {
    return new ark::unittest::HeapsTestCase();
}
