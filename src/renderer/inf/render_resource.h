#ifndef ARK_RENDERER_INF_RENDER_RESOURCE_H_
#define ARK_RENDERER_INF_RENDER_RESOURCE_H_

#include <functional>

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderResource {
public:
    typedef std::function<void(GraphicsContext&)> Recycler;

    virtual ~RenderResource() = default;

    virtual uint32_t id() = 0;

    virtual void upload(GraphicsContext& graphicsContext) = 0;
    virtual Recycler recycle() = 0;
};

}

#endif
