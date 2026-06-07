#pragma once

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/render_view.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"

namespace ark::plugin::vulkan {

class RenderViewVulkan final : public RenderView {
public:
    RenderViewVulkan(const sp<VKRenderer>& renderer, sp<RenderController> renderController);

    void onSurfaceCreated() override;
    void onSurfaceChanged(uint32_t width, uint32_t height) override;
    void onRenderFrame(const V4& backgroundColor, RenderCommand& renderCommand) override;

    sp<Bitmap> doScreenshot() override;

private:
    sp<VKRenderer> _renderer;
    sp<VKGraphicsContext> _vk_graphics_context;
    sp<VKComputeContext> _vk_compute_context;
    sp<RenderController> _render_controller;
};

}