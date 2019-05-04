#ifndef ARK_RENDERER_IMPL_RENDER_VIEW_RENDER_VIEW_OPENGL_H_
#define ARK_RENDERER_IMPL_RENDER_VIEW_RENDER_VIEW_OPENGL_H_

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"

namespace ark {

class RenderViewOpenGL : public RenderView {
public:
    RenderViewOpenGL(const sp<RenderContext>& renderContext, const sp<RenderController>& renderController);
    ~RenderViewOpenGL() override;

    virtual void onSurfaceCreated() override;
    virtual void onSurfaceChanged(uint32_t width, uint32_t height) override;
    virtual void onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand) override;

private:
    void initialize(uint32_t width, uint32_t height);
    
private:
    op<GraphicsContext> _graphics_context;
};

}

#endif
