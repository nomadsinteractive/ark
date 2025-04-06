#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

#include "vulkan/forwarding.h"

namespace ark::plugin::vulkan {

class VKFramebufferRenderer final : public Renderer {
public:
    VKFramebufferRenderer(sp<Renderer> renderer, sp<VKFramebuffer> framebuffer);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

private:
    sp<Renderer> _renderer;
    sp<DrawDecorator> _draw_decorator;
};

}
