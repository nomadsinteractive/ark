#ifndef ARK_RENDERER_INF_RENDER_VIEW_FACTORY_H_
#define ARK_RENDERER_INF_RENDER_VIEW_FACTORY_H_

#include "graphics/base/viewport.h"

#include "graphics/inf/render_view.h"

namespace ark {

class ARK_API RenderViewFactory {
public:
    virtual ~RenderViewFactory() = default;

    virtual sp<RenderView> createRenderView(const Viewport& viewport) = 0;
};

}

#endif
