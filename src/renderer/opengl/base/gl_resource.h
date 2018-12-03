#ifndef ARK_RENDERER_OPENGL_BASE_GL_RESOURCE_H_
#define ARK_RENDERER_OPENGL_BASE_GL_RESOURCE_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"

namespace ark {

class ARK_API GLResource : public RenderResource {
public:
    GLResource();
    virtual ~GLResource() = default;

    uint32_t id() const;

protected:
    uint32_t _id;
};

}

#endif
