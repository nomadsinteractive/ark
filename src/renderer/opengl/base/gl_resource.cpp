#include "renderer/opengl/base/gl_resource.h"

namespace ark {

GLResource::GLResource()
    : _id(0)
{
}

uint32_t GLResource::id() const
{
    return _id;
}

}
