#include "opengl/util/gl_debug.h"

#include <png.h>
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

}
