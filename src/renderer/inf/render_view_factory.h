#ifndef ARK_RENDERER_INF_RENDER_VIEW_FACTORY_H_
#define ARK_RENDERER_INF_RENDER_VIEW_FACTORY_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderViewFactory {
public:
    virtual ~RenderViewFactory() = default;

    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) = 0;
};

}

#endif
