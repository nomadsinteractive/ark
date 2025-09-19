#include "core/impl/writable/writable_memory.h"

namespace ark {

WritableMemory::WritableMemory(void* ptr)
    : _ptr(static_cast<uint8_t*>(ptr))
{
}

uint32_t WritableMemory::write(const void* buffer, const uint32_t size, const uint32_t offset)
{
    memcpy(_ptr + offset, buffer, size);
    return size;
}

}
