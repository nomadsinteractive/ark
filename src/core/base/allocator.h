#ifndef ARK_CORE_BASE_ALLOCATOR_H_
#define ARK_CORE_BASE_ALLOCATOR_H_

#include <mutex>
#include <vector>

#include "core/base/api.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Allocator {
public:
    Allocator(const sp<MemoryPool>& memoryPool, size_t blockSize = 1024 * 1024);

    ByteArray::Borrowed sbrk(size_t size, size_t alignment = sizeof(void*));

private:
    void newBlock(size_t size);

private:
    sp<MemoryPool> _memory_pool;
    size_t _block_size;

    std::vector<bytearray> _blocks;
    uint8_t* _ptr;
    size_t _available;

    std::mutex _mutex;
};

}

#endif
