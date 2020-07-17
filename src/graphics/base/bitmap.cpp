#include "graphics/base/bitmap.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <algorithm>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/array.h"

#include "graphics/base/bitmap_bundle.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

Bitmap::Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels, bool allocate)
    : Bitmap(width, height, rowBytes, channels, allocate ? sp<ByteArray::Allocated>::make(height * rowBytes) : nullptr)
{
}

Bitmap::Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels, const bytearray& bytes)
    : _width(width), _height(height), _row_bytes(rowBytes), _channels(channels), _bytes(bytes)
{
    DASSERT(_channels != 0);
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
    Bitmap s(w, h, w * _channels * d, _channels, true);
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
    Bitmap s(w, h, w * _channels * d, _channels, true);
    for(uint32_t i = 0; i < h; ++i)
        memcpy(s.at(0, i), at(x, i), s.rowBytes());
    return s;
}

void Bitmap::draw(void* buf, uint32_t width, uint32_t height, int32_t x, int32_t y, uint32_t stride)
{
    if(buf == nullptr || width == 0 || height == 0)
        return;

    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    uint32_t x_offset = x < 0 ? 0 - x : 0;
    uint32_t y_offset = y < 0 ? 0 - y : 0;
    uint32_t x_max = std::min(x + width, _width);
    uint32_t y_max = std::min(y + height, _height);
    uint8_t* row = at(0, y);
    uint32_t i = y + y_offset;
    uint32_t srcComponentSize = stride / width;
    uint32_t dstComponentSize = _row_bytes / _width;

    DCHECK(dstComponentSize >= srcComponentSize, "Src component size cannot be greater than dest component size");

    for (i = y; i < y_max; i++, row += _row_bytes)
        if(srcComponentSize == dstComponentSize)
        {
            uint32_t copylen = (x_max - x - x_offset) * srcComponentSize;
            memcpy(row + (x + x_offset) * dstComponentSize, reinterpret_cast<const void*>(ptr + (i - y - y_offset) * stride), copylen);
        }
        else
            for(uint32_t j = x + x_offset; j < x_max; ++j)
            {
                memcpy(row + j * dstComponentSize, reinterpret_cast<const void*>((ptr + j * srcComponentSize) + (i - y - y_offset) * stride), srcComponentSize);
                memset(row + j * dstComponentSize + srcComponentSize, 0xff, dstComponentSize - srcComponentSize);
            }
}

void Bitmap::dump(const String& filename) const
{
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = _channels;
    int depth = 8;

    FILE* fp = fopen(filename.c_str(), "wb");
    if(!fp)
        return;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr != NULL)
    {
        png_infop info_ptr = png_create_info_struct(png_ptr);

        /* Set up error handling. */

        if(info_ptr != NULL && !setjmp(png_jmpbuf(png_ptr)))
        {
            /* Set image attributes. */
            uint32_t x, y;

            int colorType = pixel_size == 1 ? PNG_COLOR_TYPE_GRAY : (pixel_size == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA);
            png_set_IHDR(png_ptr,
                         info_ptr,
                         _width,
                         _height,
                         depth,
                         colorType,
                         PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_DEFAULT,
                         PNG_FILTER_TYPE_DEFAULT);

            /* Initialize rows of PNG. */

            png_byte** row_pointers = (png_byte**) png_malloc(png_ptr, _height * sizeof(png_byte*));
            for(y = 0; y < _height; ++y)
            {
                png_byte* row = (png_byte*)
                    png_malloc(png_ptr, sizeof(uint8_t) * _width * pixel_size);
                row_pointers[y] = row;
                for(x = 0; x < _width; ++x)
                {
                    const uint8_t* pixel = at(x, y);
                    for(int i = 0; i < pixel_size; i++)
                        *row++ = pixel[i];
                }
            }

            /* Write the image data to "fp". */

            png_init_io(png_ptr, fp);
            png_set_rows(png_ptr, info_ptr, row_pointers);
            png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

            for(y = 0; y < _height; y++)
            {
                png_free(png_ptr, row_pointers[y]);
            }
            png_free(png_ptr, row_pointers);
        }
        png_destroy_write_struct(&png_ptr, &info_ptr);
    }
    fclose(fp);
}

Bitmap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _src(factory.ensureBuilder<String>(manifest, Constants::Attributes::SRC)), _bitmap_bundle(resourceLoaderContext->bitmapBundle())
{
}

sp<Bitmap> Bitmap::BUILDER::build(const Scope& args)
{
    return _bitmap_bundle->get(_src->build(args));
}

}


