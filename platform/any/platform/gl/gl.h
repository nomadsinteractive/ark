#pragma once

#include "platform/gl/_gl_internal.h"

namespace ark {

constexpr GLenum GLIndexType = sizeof(element_index_t) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

#ifdef ARK_FLAG_DEBUG
#define GL_CHECK_ERROR(x) do { x; const GLenum err = glGetError(); CHECK(err == GL_NO_ERROR, "OpenGL error: 0x%x", err); } while(false)
#else
#define GL_CHECK_ERROR(x) x
#endif

}
