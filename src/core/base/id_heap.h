#ifndef ARK_CORE_BASE_ID_HEAP_H_
#define ARK_CORE_BASE_ID_HEAP_H_

#include <vector>

#include "core/base/api.h"
#include "core/base/heap.h"

namespace ark {

class ARK_API IDHeap {
public:
    IDHeap(size_t heapSizeL1, size_t heapSizeL2 = 0);

//  [[script::bindings::auto]]
    size_t allocate(size_t size, size_t alignment = 1);
//  [[script::bindings::auto]]
    void free(size_t ptr);

private:
    class IDMemory {
    public:
        IDMemory(size_t begin, size_t end);

        size_t begin() const;
        size_t end() const;

    private:
        size_t _begin;
        size_t _end;
    };

private:
    typedef Heap<IDMemory, size_t, 1> HeapType;

    HeapType _heap;
};

}

#endif
