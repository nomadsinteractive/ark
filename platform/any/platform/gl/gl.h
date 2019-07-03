#ifndef _PLATFORM_GL_H
#define _PLATFORM_GL_H

#include "platform/gl/_gl_internal.h"

namespace ark {

const GLenum GLIndexType = sizeof(element_index_t) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

}

#endif
