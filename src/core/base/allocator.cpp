#include "core/base/allocator.h"

#include "core/base/memory_pool.h"

namespace ark {

Allocator::Allocator(const sp<MemoryPool>& memoryPool, size_t blockSize)
    : _memory_pool(memoryPool), _block_size(blockSize), _ptr(nullptr), _available(0)
{
}

ByteArray::Borrowed Allocator::sbrk(size_t size, size_t alignment)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    size_t m = size % alignment;
    size_t sizeNeeded = size + (m ? alignment - m : 0);

    if(_available < sizeNeeded)
        newBlock(std::max(sizeNeeded, _block_size));

    uint8_t* ptr = _ptr;
    _available -= sizeNeeded;
    _ptr += sizeNeeded;
    return ByteArray::Borrowed(ptr, size);
}

void Allocator::newBlock(size_t size)
{
    bytearray block = _memory_pool ? _memory_pool->allocate(size) : bytearray::adopt(new ByteArray::Allocated(size));
    _ptr = block->buf();
    _available = block->length();
    _blocks.push_back(std::move(block));
}

}
