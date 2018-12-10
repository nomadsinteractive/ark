#ifndef ARK_RENDERER_OPENGL_BASE_GL_RESOURCE_H_
#define ARK_RENDERER_OPENGL_BASE_GL_RESOURCE_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API GLResource : public Resource {
public:
    virtual ~GLResource() = default;
};

}

#endif
