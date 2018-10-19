#ifndef ARK_CORE_BASE_MEMORY_POOL_H_
#define ARK_CORE_BASE_MEMORY_POOL_H_

#include <atomic>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API MemoryPool {
private:
    enum {
        MAX_BLOCK_COUNT = 18,
        BLOCK_SIZE_BASE_LOG2 = 4,
        SUB_BLOCK_BITS = 2,
        SUB_BLOCK_COUNT = SUB_BLOCK_BITS << 1,
        PAGE_TTL_INCREMENT = 4
    };

    class Slot : public Array<uint8_t> {
    public:
        Slot(size_t size);

        virtual uint8_t* buf();
        virtual size_t length();

        void setLength(size_t length);

    private:
        bytearray _delegate;
        size_t _length;
    };

    struct Page {
        Page(size_t size, int32_t ttl);

        sp<Slot> obtain(size_t size);
        void recycle(sp<Slot> slot);

        std::vector<sp<Slot>> _slots;
        size_t _size;
        int32_t _ttl;
    };

    class Block {
    public:
        Block(size_t pageSize, int32_t pageTtl);

        sp<Slot> obtain(size_t size);
        void recycle(sp<Slot> slot);

    private:
        sp<Page> lockPage();

    private:
        LockFreeStack<sp<Page>> _pages;
        size_t _page_size;

        std::atomic<int32_t> _page_ttl;
    };

public:
    MemoryPool();

    array<uint8_t> allocate(size_t size);

private:
    size_t getBlockId(size_t size);
    size_t findmsb(size_t size) const;

private:
    std::shared_ptr<Block> _blocks_v2[MAX_BLOCK_COUNT * SUB_BLOCK_COUNT];
};

}

#endif
