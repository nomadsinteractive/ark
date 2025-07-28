#include "opengl/util/gl_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "graphics/base/bitmap.h"
#include "opengl/base/gl_pipeline.h"
#include "renderer/base/graphics_context.h"

namespace ark::plugin::opengl {

int32_t GLDebug::glGetBufferSize(GLenum target)
{
    int32_t nBufferSize = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &nBufferSize);
    return nBufferSize;
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

}
