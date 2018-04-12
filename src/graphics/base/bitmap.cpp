#include "graphics/base/bitmap.h"

#include <algorithm>

#include "core/impl/array/dynamic_array.h"

namespace ark {

Bitmap::Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels)
    : Bitmap(width, height, rowBytes, channels, rowBytes ? sp<DynamicArray<uint8_t>>::make(height * rowBytes) : nullptr)
{
}

Bitmap::Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels, const bytearray& bytes)
    : _width(width), _height(height), _row_bytes(rowBytes), _channels(channels), _bytes(bytes)
{
}

Bitmap::~Bitmap()
{
}

uint32_t Bitmap::width() const
{
    return _width;
}

uint32_t Bitmap::height() const
{
    return _height;
}

uint8_t Bitmap::channels() const
{
    return _channels;
}

uint32_t Bitmap::rowBytes() const
{
    return _row_bytes;
}

const sp<Array<uint8_t>>& Bitmap::bytes() const
{
    return _bytes;
}

uint8_t* Bitmap::at(uint32_t x, uint32_t y) const
{
    return _bytes ? _bytes->buf() + y * _row_bytes + x * _channels : nullptr;
}

void Bitmap::draw(void* buf, uint32_t width, uint32_t height, int32_t x, int32_t y, int32_t stride)
{
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    uint32_t x_offset = x < 0 ? 0 - x : 0;
    uint32_t y_offset = y < 0 ? 0 - y : 0;
    uint32_t x_max = std::min(x + width, _width);
    uint32_t y_max = std::min(y + height, _height);
    uint8_t* row = at(0, y);
    uint32_t i = y + y_offset;
    uint32_t copylen = x_max - x - x_offset;

    for (i = y; i < y_max; i++, row += _row_bytes)
        memcpy(row + x + x_offset, reinterpret_cast<const void*>(ptr + (i - y - y_offset) * stride), copylen);
}

}


