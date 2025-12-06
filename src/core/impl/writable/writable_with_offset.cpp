#include "core/impl/writable/writable_with_offset.h"

namespace ark {

WritableWithOffset::WritableWithOffset(Writable& writable, const size_t offset)
    : _writable(writable), _offset(offset)
{
}

uint32_t WritableWithOffset::write(const void* buffer, const uint32_t size, const uint32_t offset)
{
    return _writable.write(buffer, size, offset + _offset);
}

void WritableWithOffset::setOffset(const size_t offset)
{
    _offset = offset;
}

}
