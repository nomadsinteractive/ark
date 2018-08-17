#ifndef _PLATFORM_GL_H
#define _PLATFORM_GL_H

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>

using namespace gl;

namespace ark {

const GLenum GLIndexType = sizeof(glindex_t) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

}

#endif
