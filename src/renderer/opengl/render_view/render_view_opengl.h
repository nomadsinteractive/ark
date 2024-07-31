#pragma once

#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"

namespace ark {

class RenderViewOpenGL final : public RenderView {
public:
    RenderViewOpenGL(sp<RenderEngineContext> renderContext, sp<RenderController> renderController);
    ~RenderViewOpenGL() override;

    void onSurfaceCreated() override;
    void onSurfaceChanged(uint32_t width, uint32_t height) override;
    void onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand) override;

private:
    void initialize(uint32_t width, uint32_t height);
    
private:
    op<GraphicsContext> _graphics_context;
};

}
