#include "renderer/opengl/util/gl_debug.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "graphics/base/bitmap.h"
#include "graphics/base/matrix.h"
#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/graphics_context.h"

namespace ark {

int32_t GLDebug::glGetBufferSize(GLenum target)
{
    int32_t nBufferSize = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &nBufferSize);
    return nBufferSize;
}

GLuint GLDebug::glTestIndexBuffer()
{
    GLuint id = 0;
    const element_index_t indices[] = { 0, 1, 2 };
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6, indices, GL_DYNAMIC_DRAW);
    return id;
}

GLuint GLDebug::glTestArrayBuffer()
{
    GLuint id = 0;
//    const GLfloat arrays[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
    const GLfloat arrays[] = { 200.0f, -0.5f, 0.0f, 0.0f, 200.0f, 0.0f, -0.5f, -0.5f, 0.0f };
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, 36, arrays, GL_DYNAMIC_DRAW);
    return id;
}

void GLDebug::glPrintString(const char* name, GLenum value)
{
    LOGD("%s: %s", name, glGetString(value));
}

void GLDebug::glPrintInteger(const char* name, GLenum value)
{
    GLint data = 0;
    glGetIntegerv(value, &data);
    LOGD("%s: %d", name, data);
}

void GLDebug::glCheckError(const char* tag)
{
    for (GLenum error = glGetError(); error != 0; error = glGetError()) {
        LOGE("%s glError (0x%x)", tag, error);
    }
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
   success, non-zero on error. */
void GLDebug::glDumpBitmapToPngFile(const Bitmap* bitmap, const char* filename)
{
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = bitmap->channels();
    int depth = 8;

    FILE* fp = fopen(filename, "wb");
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
                         bitmap->width(),
                         bitmap->height(),
                         depth,
                         colorType,
                         PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_DEFAULT,
                         PNG_FILTER_TYPE_DEFAULT);

            /* Initialize rows of PNG. */

            png_byte** row_pointers = (png_byte**) png_malloc(png_ptr, bitmap->height() * sizeof(png_byte*));
            for(y = 0; y < bitmap->height(); ++y)
            {
                png_byte* row = (png_byte*)
                    png_malloc(png_ptr, sizeof(uint8_t) * bitmap->width() * pixel_size);
                row_pointers[y] = row;
                for(x = 0; x < bitmap->width(); ++x)
                {
                    const uint8_t* pixel = bitmap->at(x, y);
                    for(int i = 0; i < pixel_size; i++)
                        *row++ = pixel[i];
                }
            }

            /* Write the image data to "fp". */

            png_init_io(png_ptr, fp);
            png_set_rows(png_ptr, info_ptr, row_pointers);
            png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

            for(y = 0; y < bitmap->height(); y++)
            {
                png_free(png_ptr, row_pointers[y]);
            }
            png_free(png_ptr, row_pointers);
        }
        png_destroy_write_struct(&png_ptr, &info_ptr);
    }
    fclose(fp);
}

}
