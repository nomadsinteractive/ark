#ifndef ARK_CORE_BASE_ALLOCATOR_H_
#define ARK_CORE_BASE_ALLOCATOR_H_

#include <vector>

#include "core/base/api.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Allocator {
public:
    Allocator(const sp<MemoryPool>& memoryPool, size_t blockSize = 1024 * 1024, size_t alignment = sizeof(void*));

    ByteArray::Borrowed sbrk(size_t size);

private:
    void newBlock(size_t size);

private:
    sp<MemoryPool> _memory_pool;
    size_t _block_size;
    size_t _alignment;

    std::vector<bytearray> _blocks;
    uint8_t* _ptr;
    size_t _available;
};

}

#endif
