#ifndef ARK_GRAPHICS_INF_RENDER_VIEW_H_
#define ARK_GRAPHICS_INF_RENDER_VIEW_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class RenderView {
public:
    virtual ~RenderView() = default;

    virtual void onSurfaceCreated() = 0;
    virtual void onSurfaceChanged(uint32_t width, uint32_t height) = 0;
    virtual void onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand) = 0;

};

}

#endif
