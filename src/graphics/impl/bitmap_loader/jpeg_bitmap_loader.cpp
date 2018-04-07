#include "graphics/impl/bitmap_loader/jpeg_bitmap_loader.h"

#ifdef ARK_USE_LIBJPEG_TURBO

#include <stdio.h>
#include <jpeglib.h>

#include "core/inf/readable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"

#define INPUT_BUF_SIZE 4096

namespace ark {

namespace {

struct my_source_mgr {
    struct jpeg_source_mgr pub;                   /* public fields */
    sp<Readable> _readable;                       /* source stream */
    bool _start_of_file;                          /* have we gotten any data yet? */
    JOCTET _buffer[INPUT_BUF_SIZE];               /* start of buffer */

    my_source_mgr(const sp<Readable>& readable)
        : _readable(readable) {
    }
};

static void init_source (j_decompress_ptr cinfo)
{
    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);
    src->_start_of_file = true;
}

static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);
    size_t nbytes = src->_readable->read(src->_buffer, sizeof(src->_buffer));

    if(nbytes == 0)
    {
        DCHECK(!src->_start_of_file, "Error reading JPEG data");
        src->_buffer[0] = (JOCTET) 0xFF;
        src->_buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }

    src->pub.next_input_byte = src->_buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->_start_of_file = false;
    return TRUE;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);
    long seek = num_bytes - src->pub.bytes_in_buffer;

    if(seek <= 0)
    {
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
    else
    {
        src->_readable->seek(seek, SEEK_CUR);
        src->pub.bytes_in_buffer = 0;
        src->pub.next_input_byte = nullptr;
    }
}

static void term_source(j_decompress_ptr /*cinfo*/)
{
}

}

JPEGBitmapLoader::JPEGBitmapLoader(bool justDecodeBounds)
    : _just_decode_bounds(justDecodeBounds)
{
}

bitmap JPEGBitmapLoader::load(const sp<Readable>& readable)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr err_msg = {0};


    cinfo.err = jpeg_std_error(&err_msg);
    err_msg.error_exit = nullptr;
    jpeg_create_decompress(&cinfo);

    my_source_mgr src(readable);

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     */
    cinfo.src = reinterpret_cast<jpeg_source_mgr*>(&src);

    src.pub.init_source = init_source;
    src.pub.fill_input_buffer = fill_input_buffer;
    src.pub.skip_input_data = skip_input_data;
    src.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src.pub.term_source = term_source;
    src.pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src.pub.next_input_byte = nullptr; /* until buffer loaded */

    jpeg_read_header(&cinfo, TRUE);

    int32_t row_stride = cinfo.image_width * cinfo.num_components;
    const bitmap buf = bitmap::make(cinfo.image_width, cinfo.image_height, _just_decode_bounds ? 0 : row_stride, static_cast<uint8_t>(cinfo.num_components));

    if(!_just_decode_bounds)
    {
        uint8_t* bufptr = buf->at(0, 0);

        jpeg_start_decompress(&cinfo);

        while(cinfo.output_scanline < cinfo.output_height) {
            JSAMPROW scanline = reinterpret_cast<JSAMPROW>(bufptr);
            jpeg_read_scanlines(&cinfo, &scanline, 1);
            bufptr += row_stride;
        }

        jpeg_finish_decompress(&cinfo);
    }

    jpeg_destroy_decompress(&cinfo);

    return buf;
}

}

#endif
