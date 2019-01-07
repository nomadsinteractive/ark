#include "core/base/memory_pool.h"

#include "core/impl/array/aligned_dynamic_array.h"
#include "core/util/math.h"
#include "core/util/log.h"

namespace ark {

MemoryPool::MemoryPool()
{
    for(size_t i = 0; i < MAX_BLOCK_COUNT; ++i)
    {
        size_t blockSize = i ? 1 << (BLOCK_SIZE_BASE_LOG2 - 1 + i) : 0;
        for(size_t j = 0; j < SUB_BLOCK_COUNT; ++j)
        {
            size_t slotSize = blockSize + (1 << (BLOCK_SIZE_BASE_LOG2 - SUB_BLOCK_BITS + (i ? i - 1 : 0))) * (j + 1);
            size_t blockid = i * SUB_BLOCK_COUNT + j;
            DASSERT(blockid == getBlockId(slotSize));
            _blocks[blockid] = std::make_shared<Block>(slotSize, 1);
        }
    }
}

bytearray MemoryPool::allocate(size_t size) const
{
    size_t blockId = getBlockId(size);
    DCHECK(blockId < MAX_BLOCK_COUNT * SUB_BLOCK_COUNT, "Cannot allocate memory, size too big %d", size);
    const std::shared_ptr<Block> block = _blocks[blockId];
    const sp<Slot> slot = block->obtain(size);
    const bytearray allocated(slot.get(), slot.interfaces(), [block, slot](ByteArray*) {
            block->recycle(slot);
        });
    return allocated;
}

size_t MemoryPool::getBlockId(size_t size) const
{
    DASSERT(size > 0);

    size_t s = (size - 1) >> (BLOCK_SIZE_BASE_LOG2 - SUB_BLOCK_BITS);

    if(s < (1 << SUB_BLOCK_BITS))
        return s;

    size_t m = findmsb(s);
    size_t p = ((s >> (m - SUB_BLOCK_BITS - 1)) - (1 << SUB_BLOCK_BITS));
    size_t r = (m - SUB_BLOCK_BITS) * (1 << SUB_BLOCK_BITS) + p;
    return r;
}

size_t MemoryPool::findmsb(size_t size) const
{
    for(size_t i = 0; i < sizeof(size_t) * 8; ++i)
        if((size >> i) == 0)
            return i;

    DFATAL("");
    return 0;
}

MemoryPool::Slot::Slot(size_t size)
    : _delegate(sp<AlignedDynamicArray<uint8_t, uint32_t>>::make(size)), _length(0)
{
}

uint8_t* MemoryPool::Slot::buf()
{
    return _delegate->buf();
}

size_t MemoryPool::Slot::length()
{
    return _length;
}

void MemoryPool::Slot::setLength(size_t length)
{
    _length = length;
}

MemoryPool::Page::Page(size_t size, int32_t ttl)
    : _size(size), _ttl(ttl)
{
}

sp<MemoryPool::Slot> MemoryPool::Page::obtain(size_t size)
{
    DCHECK(size <= _size, "slot-size: %d, allocation size: %d", _size, size);
    sp<MemoryPool::Slot> s;
    if(_slots.empty())
        s = sp<Slot>::make(_size);
    else
    {
        s = _slots.back();
        _slots.pop_back();
    }
    s->setLength(size);
    ++_ttl;
    return s;
}

void MemoryPool::Page::recycle(sp<MemoryPool::Slot> slot)
{
    _slots.push_back(std::move(slot));
    --_ttl;
}

MemoryPool::Block::Block(size_t pageSize, int32_t pageTtl)
    : _page_size(pageSize), _page_ttl(pageTtl)
{
}

sp<MemoryPool::Slot> MemoryPool::Block::obtain(size_t size)
{
    const sp<Page> page = lockPage();
    const sp<MemoryPool::Slot> slot = page->obtain(size);
    _pages.push(page);
    return slot;
}

void MemoryPool::Block::recycle(sp<MemoryPool::Slot> slot)
{
    const sp<Page> page = lockPage();
    page->recycle(std::move(slot));
    if(page->_ttl >= 0)
        _pages.push(page);
    else
    {
        _page_ttl -= 1;
        LOGD("Dropping page, page-size: %d page-ttl: %d", _page_size, _page_ttl.load(std::memory_order_relaxed));
    }
}

sp<MemoryPool::Page> MemoryPool::Block::lockPage()
{
    sp<Page> page;
    if(!_pages.pop(page))
    {
        _page_ttl += PAGE_TTL_INCREMENT;
        int32_t pageTTL = _page_ttl.load(std::memory_order_relaxed) / PAGE_TTL_INCREMENT;
        page = sp<Page>::make(_page_size, pageTTL);
        LOGD("Creating page, page-size: %d page-ttl: %d", _page_size, pageTTL);
    }
    return page;
}

}
