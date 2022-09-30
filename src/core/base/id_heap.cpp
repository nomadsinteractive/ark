#include "core/base/id_heap.h"

namespace ark {

IDHeap::IDHeap(size_t heapSizeL1, size_t heapSizeL2)
    : _heap(IDMemory(0, heapSizeL1), sp<HeapType::L1>::make(8))
{
    if(heapSizeL2 > 0)
        _heap.extend(IDMemory(heapSizeL1, heapSizeL1 + heapSizeL2), sp<HeapType::L2>::make());
}

size_t IDHeap::allocate(size_t size, size_t alignment)
{
    Optional<size_t> opt = _heap.allocate(size, alignment);
    CHECK(opt, "Allocate heap failed, size requested: %lld, alignment: %lld", size, alignment);
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
