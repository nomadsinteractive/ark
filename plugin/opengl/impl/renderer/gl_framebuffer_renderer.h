#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_target.h"

#include "opengl/forwarding.h"

namespace ark::plugin::opengl {

class GLFramebufferRenderer final : public Renderer {
public:
    GLFramebufferRenderer(sp<GLFramebuffer> fbo, int32_t width, int32_t height, sp<Renderer> renderer, RenderTarget::ClearBitSet clearBits);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

private:
    sp<Renderer> _renderer;
    sp<RenderCommand> _pre_draw;
    sp<RenderCommand> _post_draw;
};

}
