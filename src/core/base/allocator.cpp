#include "core/base/allocator.h"

namespace ark {

Allocator::Allocator(sp<Pool> pool)
    : _pool(std::move(pool)), _block_size(_pool ? _pool->_block_size : 128 * 1024)
{
}

Allocator::~Allocator()
{
    if(_pool)
    {
        for(sp<Block>& i : _actived.clear())
            _pool->recycle(std::move(i));
        for(sp<Block>& i : _allocated.clear())
            _pool->recycle(std::move(i));
    }
}

uint8_t* Allocator::sbrk(size_t size, size_t alignment)
{
    size_t m = size % alignment;
    size_t sizeNeeded = size + (m ? alignment - m : 0);

    uint8_t* ptr = _sbrk(sizeNeeded);
    size_t mod = reinterpret_cast<size_t>(ptr) % alignment;
    if(mod != 0)
        ptr += (alignment - mod);

    return ptr;
}

uint8_t* Allocator::_sbrk(size_t size)
{
    ASSERT(size < _block_size);

    while(true) {

        sp<Block> locked;
        if(!_actived.pop(locked))
            locked = _pool ? _pool->obtain() : sp<Block>::make(_block_size);

        uint8_t* ptr = locked->allocate(size);
        if(ptr)
        {
            _actived.push(std::move(locked));
            return ptr;
        }

        _allocated.push(std::move(locked));
    }
}

ByteArray::Borrowed Allocator::sbrkSpan(size_t size, size_t alignment)
{
    return {sbrk(size, alignment), size};
}

Allocator::Block::Block(size_t blockSize)
    : _memory(blockSize), _allocated_ptr(&_memory.front())
{
}

uint8_t* Allocator::Block::allocate(size_t size)
{
    const uint8_t* const end = &_memory.back() + 1;
    if(_allocated_ptr + size > end)
        return nullptr;

    uint8_t* ptr = _allocated_ptr;
    _allocated_ptr += size;
    return ptr;
}

void Allocator::Block::reset()
{
    _allocated_ptr = &_memory.front();
}

Allocator::Pool::Pool(size_t blockSize)
    : _block_size(blockSize)
{
}

sp<Allocator::Block> Allocator::Pool::obtain()
{
    sp<Block> block;
    if(_blocks.pop(block))
        block->reset();
    else
        block = sp<Block>::make(_block_size);
    return block;
}

void Allocator::Pool::recycle(sp<Block> block)
{
    _blocks.push(std::move(block));
}

}
