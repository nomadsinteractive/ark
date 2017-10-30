#ifndef ARK_RENDERER_BASE_GL_RESOURCE_H_
#define ARK_RENDERER_BASE_GL_RESOURCE_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLResource {
public:
    virtual ~GLResource() = default;

    virtual uint32_t id() = 0;
    virtual void prepare(GraphicsContext& graphicsContext) = 0;
    virtual void recycle(GraphicsContext& graphicsContext) = 0;
};

}

#endif
