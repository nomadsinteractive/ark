#pragma once

#include "graphics/inf/render_view.h"

namespace ark::plugin::bgfx {

class RenderViewBgfx final : public RenderView {
public:
    void onSurfaceCreated() override;

    void onSurfaceChanged(uint32_t width, uint32_t height) override;

    void onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand) override;
};

}
