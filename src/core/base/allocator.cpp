#include "core/base/allocator.h"

namespace ark {

struct Allocator::Chunk {

    Chunk(const size_t chunkSize)
        : _memory(chunkSize), _allocated_ptr(_memory.data())
    {
    }

    uint8_t* allocate(const size_t size)
    {
        if(const uint8_t* const end = &_memory.back() + 1; _allocated_ptr + size > end)
            return nullptr;

        uint8_t* ptr = _allocated_ptr;
        _allocated_ptr += size;
        return ptr;
    }

    Vector<uint8_t> _memory;
    uint8_t* _allocated_ptr;
};

Allocator::Allocator()
    : _default_chunk_size(64 * 1024)
{
}

Allocator::~Allocator()
{
}

uint8_t* Allocator::sbrk(const size_t size, const size_t alignment)
{
    const size_t m = size % alignment;
    const size_t sizeNeeded = size + (m ? alignment - m : 0);

    uint8_t* ptr = _sbrk(sizeNeeded);
    if(const size_t mod = reinterpret_cast<size_t>(ptr) % alignment)
        ptr += (alignment - mod);

    return ptr;
}

uint8_t* Allocator::_sbrk(const size_t size)
{
    if(size > _default_chunk_size)
    {
        sp<Chunk> chunk = sp<Chunk>::make(size);
        uint8_t* ptr = chunk->allocate(size);
        _allocated.push(std::move(chunk));
        return ptr;
    }

    while(true)
    {
        Optional<sp<Chunk>> optLocked = _actived.pop();
        sp<Chunk> locked = optLocked ? std::move(optLocked.value()) : sp<Chunk>::make(_default_chunk_size);
        if(uint8_t* ptr = locked->allocate(size))
        {
            _actived.push(std::move(locked));
            return ptr;
        }

        _allocated.push(std::move(locked));
    }
}

ByteArray::Borrowed Allocator::sbrkSpan(size_t size, const size_t alignment)
{
    return {sbrk(size, alignment), size};
}

}
