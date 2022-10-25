#include "core/base/id_heap.h"

namespace ark {

IDHeap::IDHeap(size_t heapSize, size_t heapSizeL1, size_t chunkSizeL1)
{
    CHECK(heapSize != 0 && heapSizeL1 <= heapSize && (heapSizeL1 == 0 || (chunkSizeL1 != 0 && chunkSizeL1 <= heapSizeL1)), "Illegal heap creating arugments, heap_size: %zu, heap_size_l1: %zu, chunk_size_l1: %zu", heapSize, heapSizeL1, chunkSizeL1);
    size_t heapSizeL2 = heapSize - heapSizeL1;
    if(heapSizeL1 > 0)
        _heap.extend(IDMemory(0, heapSizeL1), sp<HeapType::L1>::make(chunkSizeL1));
    if(heapSizeL2 > 0)
        _heap.extend(IDMemory(heapSizeL1, heapSize), sp<HeapType::L2>::make());
}

size_t IDHeap::allocate(size_t size, size_t alignment)
{
    Optional<size_t> opt = _heap.allocate(size, alignment);
    CHECK(opt, "Allocate heap failed, size requested: %zu, alignment: %zu", size, alignment);
    return opt.value();
}

void IDHeap::free(size_t ptr)
{
    _heap.free(ptr);
}

IDHeap::IDMemory::IDMemory(size_t begin, size_t end)
    : _begin(begin), _end(end)
{
}

size_t IDHeap::IDMemory::begin() const
{
    return _begin;
}

size_t IDHeap::IDMemory::end() const
{
    return _end;
}

}
