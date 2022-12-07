#include "graphics/impl/bitmap_loader/png_bitmap_loader.h"

#ifdef ARK_USE_LIBPNG

#include <stdlib.h>

#include <png.h>

#include "core/inf/readable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"

namespace ark {

static void _png_readable_read_fn(png_structp png_ptr, png_bytep buffer, png_size_t size)
{
    Readable* readable = reinterpret_cast<Readable*>(png_get_io_ptr(png_ptr));
    readable->read(buffer, size);
}

PNGBitmapLoader::PNGBitmapLoader(bool justDecodeBounds)
    : _just_decode_bounds(justDecodeBounds)
{
}

bitmap PNGBitmapLoader::load(const sp<Readable>& readable)
{
    png_byte header[8];

    readable->read(header, 8);

    if(png_sig_cmp(header, 0, 8))
    {
        LOGE("not a PNG.");
        return nullptr;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(!png_ptr)
    {
        LOGE("png_create_read_struct returned 0.");
        return nullptr;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
        LOGE("png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return nullptr;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if(!end_info)
    {
        LOGE("png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }

    // init png reading
    png_set_read_fn(png_ptr, readable.get(), _png_readable_read_fn);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
                 nullptr, nullptr, nullptr);

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes - 1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    const bitmap bitmap =  bitmap::make(temp_width, temp_height, rowbytes, png_get_channels(png_ptr, info_ptr), !_just_decode_bounds);

    if(!_just_decode_bounds)
    {
        png_byte* image_data = (png_byte*) bitmap->at(0, 0);
        if(image_data == nullptr)
        {
            LOGE("could not allocate memory for PNG image data");
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            return nullptr;
        }

        // row_pointers is for pointing to image_data for reading the png with libpng
        png_bytep* row_pointers = (png_bytep*) malloc(temp_height * sizeof(png_bytep));
        if(row_pointers == nullptr)
        {
            LOGE("could not allocate memory for PNG row pointers");
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            return nullptr;
        }

        // set the individual row_pointers to point at the correct offsets of image_data
        for(uint32_t i = 0; i < temp_height; i++)
            row_pointers[i] = image_data + i * rowbytes;

        // read the png into image_data through row_pointers
#if !ARK_IS_BIG_ENDIAN
        png_set_swap(png_ptr);
#endif
        png_read_image(png_ptr, row_pointers);
        free(row_pointers);
    }
    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    return bitmap;
}

}

#endif
