#include "core/base/memory_pool.h"

#include "core/impl/array/aligned_dynamic_array.h"
#include "core/util/math.h"

namespace ark {

MemoryPool::SubBlock::SubBlock(uint32_t subBlockSize)
    : _preallocated(sp<LockFreeStack<array<uint8_t>>>::make()), _sub_block_size(subBlockSize), _shrink_score(0), _balanced_weight(1.0f)
{
}

array<uint8_t> MemoryPool::SubBlock::allocate()
{
    const sp<LockFreeStack<array<uint8_t>>> preallocated = _preallocated;
    bool empty = preallocated->empty();

    array<uint8_t> buf;

    if(!preallocated->pop(buf))
        buf = sp<AlignedDynamicArray<uint8_t, uint32_t>>::make(_sub_block_size);

    if(empty)
        _shrink_score += _balanced_weight;

    return array<uint8_t>(buf.get(), buf.interfaces(), [buf, preallocated](Array<uint8_t>*) {
            preallocated->push(buf);
        });
}

void MemoryPool::SubBlock::shrink()
{
    if(!_preallocated->empty())
    {
        if(_shrink_score < 1.0f)
            _preallocated->pop();
        else
        {
            _shrink_score -= 1.0f;
            _balanced_weight = 1.0f / (.1f + _shrink_score);
        }
    }
}

MemoryPool::Block::Block(uint32_t sizeLog2)
    : _size(1 << sizeLog2), _size_half(sizeLog2 > BLOCK_SIZE_LOG2 ? _size >> 1 : 0), _size_quarter_log2(sizeLog2 > BLOCK_SIZE_LOG2 ? sizeLog2 - 3 : sizeLog2 - 2)
{
    DCHECK(sizeLog2 > 2, "Invalid block size %d", 1 << sizeLog2);
    for(uint32_t i = 0; i < SUB_BLOCK_COUNT; i++)
        _sub_blocks[i].reset(new SubBlock(((i + 1) << _size_quarter_log2) + _size_half));
}

array<uint8_t> MemoryPool::Block::allocate(uint32_t size)
{
    DCHECK(size > _size_half && size <= _size, "Invalid memory size %d being allocated", size);
    uint32_t subindex = ((size - 1 - _size_half) >> _size_quarter_log2) % SUB_BLOCK_COUNT;
    return _sub_blocks[subindex]->allocate();
}

void MemoryPool::Block::shrink()
{
    for(const auto& i : _sub_blocks)
        i->shrink();
}

MemoryPool::MemoryPool(uint32_t shrinkCycle)
    : _allocation_tick(0), _shrink_cycle(shrinkCycle)
{
    for(uint32_t i = 0; i < MAX_BLOCK_COUNT; i++)
        _blocks[i].reset(new Block(i + BLOCK_SIZE_LOG2));
}

array<uint8_t> MemoryPool::allocate(uint32_t size)
{
    uint32_t idx = (size <= (1 << BLOCK_SIZE_LOG2)) ? 0 : Math::log2((size - 1) >> BLOCK_SIZE_LOG2) + 1;
    DCHECK(idx < MAX_BLOCK_COUNT, "Cannot allocate memory, size too big %d", size);
    _allocation_tick ++;
    if(_allocation_tick > _shrink_cycle)
    {
        _allocation_tick = 0;
        shrink();
    }
    return _blocks[idx]->allocate(size);
}

void MemoryPool::shrink()
{
    for(const auto& i : _blocks)
        i->shrink();
}

}
