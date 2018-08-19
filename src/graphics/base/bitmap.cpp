#include "graphics/base/bitmap.h"

#include <algorithm>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

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

uint32_t Bitmap::depth() const
{
    return _row_bytes / _channels / _width;
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

Bitmap Bitmap::resize(uint32_t w, uint32_t h) const
{
    uint32_t d = depth();
    DCHECK(d == 1 || d == 4, "Unsupported bitmap depth: %d", d);
    Bitmap s(w, h, w * _channels * d, _channels);
    if(d == 1)
        stbir_resize_uint8(_bytes->buf(), _width, _height, _row_bytes, s.at(0, 0), w, h, s.rowBytes(), _channels);
    else if (d == 4)
        stbir_resize_float(reinterpret_cast<const float*>(_bytes->buf()), _width, _height, _row_bytes, reinterpret_cast<float*>(s.at(0, 0)), w, h, s.rowBytes(), _channels);
    return s;
}

Bitmap Bitmap::crop(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const
{
    DCHECK(x + w <= width() && y + h <= height(), "Cropped image out of bounds. cropped bitmap(%d, %d, %d, %d), image size(%d, %d)", x, y, w, h, width(), height());
    uint32_t d = depth();
    Bitmap s(w, h, w * _channels * d, _channels);
    for(uint32_t i = 0; i < h; ++i)
        memcpy(s.at(0, i), at(x, i), s.rowBytes());
    return s;
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


