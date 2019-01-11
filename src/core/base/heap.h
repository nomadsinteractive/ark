#ifndef ARK_CORE_BASE_HEAP_H_
#define ARK_CORE_BASE_HEAP_H_

namespace ark {

class Heap {
public:
    Heap(size_t size);

    size_t allocate(size_t size);

private:
    struct Allocated {
        size_t _offset;
        size_t _size;
    };

private:
    size_t _size;

};

}

#endif
