#pragma once

#include "core/types/owned_ptr.h"

#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"

#include "bgfx/forwarding.h"

namespace ark::plugin::bgfx {

class RenderViewBgfx final : public RenderView {
public:
    RenderViewBgfx(sp<RenderEngineContext> renderContext, sp<RenderController> renderController);

    void onSurfaceCreated() override;
    void onSurfaceChanged(uint32_t width, uint32_t height) override;
    void onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand) override;

private:
    op<GraphicsContext> _graphics_context;
    sp<BgfxContext> _bgfx_context;
};

}
