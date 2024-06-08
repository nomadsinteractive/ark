#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Allocator {
private:
    struct Block {
        Block(size_t blockSize);

        uint8_t* allocate(size_t size);
        void reset();

        std::vector<uint8_t> _memory;
        uint8_t* _allocated_ptr;
    };

public:

    struct Pool {
        Pool(size_t blockSize = 128 * 1024);

        sp<Block> obtain();
        void recycle(sp<Block> block);

        size_t _block_size;
        LFStack<sp<Block>> _blocks;
    };

public:
    Allocator(sp<Pool> pool);
    ~Allocator();

    uint8_t* sbrk(size_t size, size_t alignment = sizeof(void*));
    ByteArray::Borrowed sbrkSpan(size_t size, size_t alignment = sizeof(void*));

private:
    uint8_t* _sbrk(size_t size);

private:
    sp<Pool> _pool;
    size_t _block_size;

    LFStack<sp<Block>> _actived;
    LFStack<sp<Block>> _allocated;
};

}
