#include "core/impl/readable/bytearray_readable.h"

#include "core/inf/array.h"

namespace ark {

BytearrayReadable::BytearrayReadable(const bytearray& array)
    : _bytearray(array), _position(0)
{
}

uint32_t BytearrayReadable::read(void* buffer, uint32_t size)
{
    uint32_t s = std::min<uint32_t>(size, remaining());
    memcpy(buffer, _bytearray->array() + _position, s);
    _position += s;
    return s;
}

int32_t BytearrayReadable::seek(int32_t position, int32_t whence)
{
    switch(whence)
    {
    case SEEK_SET:
        _position = position;
        break;
    case SEEK_CUR:
    case SEEK_END:
        _position += position;
        break;
    }
    DCHECK(_position >= 0 && _position < _bytearray->length(), "Cursor out of bounds: %d", _position);
}

int32_t BytearrayReadable::remaining()
{
    return _bytearray->length() - _position;
}

}
