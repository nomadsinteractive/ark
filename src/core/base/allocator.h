#pragma once

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Allocator {
public:
    Allocator();
    ~Allocator();

    uint8_t* sbrk(size_t size, size_t alignment = sizeof(void*));
    ByteArray::Borrowed sbrkSpan(size_t size, size_t alignment = sizeof(void*));

private:
    uint8_t* _sbrk(size_t size);

private:
    size_t _default_chunk_size;

    struct Chunk;
    LFStack<sp<Chunk>> _actived;
    LFStack<sp<Chunk>> _allocated;
};

}
