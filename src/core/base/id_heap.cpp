#include "core/base/id_heap.h"

namespace ark {

IDHeap::IDHeap(size_t heapSize)
    : _heap(IDMemory(0, heapSize))
{
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
