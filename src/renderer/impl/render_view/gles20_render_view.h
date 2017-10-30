#ifndef ARK_RENDERER_GLES20_IMPL_RENDER_VIEW_GLES20_RENDER_VIEW_H_
#define ARK_RENDERER_GLES20_IMPL_RENDER_VIEW_GLES20_RENDER_VIEW_H_

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"

namespace ark {

class GLES20RenderView : public RenderView {
public:
    GLES20RenderView(const sp<GLResourceManager>& glResources, const Viewport& viewport);
    ~GLES20RenderView();

    virtual void onSurfaceCreated() override;
    virtual void onSurfaceChanged(uint32_t width, uint32_t height) override;
    virtual void onRenderFrame(const sp<RenderCommand>& renderCommand) override;

private:
    void initialize(uint32_t width, uint32_t height);
    
private:
    sp<GLResourceManager> _gl_resource_manager;
    op<GraphicsContext> _graphics_context;
    Viewport _viewport;

};

}

#endif
