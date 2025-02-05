#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

#include "renderer/vulkan/forward.h"

namespace ark::vulkan {

class VKFramebufferRenderer final : public Renderer {
public:
    VKFramebufferRenderer(sp<Renderer> delegate, sp<VKFramebuffer> framebuffer);

    void render(RenderRequest& renderRequest, const V3& position) override;

private:
    sp<Renderer> _delegate;
    sp<VKFramebuffer> _fbo;
    sp<RenderCommand> _pre_draw;
    sp<RenderCommand> _post_draw;
};

}
