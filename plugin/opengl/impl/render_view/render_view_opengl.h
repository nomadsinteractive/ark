#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"

namespace ark::plugin::opengl {

class RenderViewOpenGL final : public RenderView {
public:
    RenderViewOpenGL(sp<RenderController> renderController);
    ~RenderViewOpenGL() override;

    void onSurfaceCreated() override;
    void onSurfaceChanged(uint32_t width, uint32_t height) override;
    void onRenderFrame(const V4& backgroundColor, RenderCommand& renderCommand) override;

    sp<Bitmap> doScreenshot() override;

private:
    void initialize(uint32_t width, uint32_t height);
    
private:
    sp<RenderController> _render_controller;
};

}
