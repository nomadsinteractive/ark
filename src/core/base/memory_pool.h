#ifndef ARK_CORE_BASE_MEMORY_POOL_H_
#define ARK_CORE_BASE_MEMORY_POOL_H_

#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/forwarding.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API MemoryPool {
private:
    enum {
        MAX_BLOCK_COUNT = 18,
        SUB_BLOCK_COUNT = 4,
        BLOCK_SIZE_LOG2 = 6
    };

    class SubBlock {
    public:
        SubBlock(uint32_t subBlockSize);

        array<uint8_t> allocate();

        void shrink();

    private:
        sp<LockFreeStack<array<uint8_t>>> _preallocated;
        uint32_t _sub_block_size;
        float _shrink_score;
        float _balanced_weight;
    };

    class Block {
    public:
        Block(uint32_t sizeLog2);

        array<uint8_t> allocate(uint32_t size);
        void shrink();

    private:
        uint32_t _size;
        uint32_t _size_half;
        uint32_t _size_quarter_log2;
        op<SubBlock> _sub_blocks[SUB_BLOCK_COUNT];
    };

public:
    MemoryPool(uint32_t shrinkCycle = 200);

    array<uint8_t> allocate(uint32_t size);

private:
    void shrink();

private:
    std::unique_ptr<Block> _blocks[MAX_BLOCK_COUNT];
    uint32_t _allocation_tick;
    uint32_t _shrink_cycle;
};

}

#endif
