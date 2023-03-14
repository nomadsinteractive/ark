#include "renderer/vulkan/base/vk_framebuffer_renderer.h"

#include "graphics/base/render_request.h"
#include "graphics/inf/render_command.h"

#include "renderer/vulkan/base/vk_framebuffer.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

namespace {

class PreDrawElementsToFBO : public RenderCommand {
public:
    PreDrawElementsToFBO(const sp<VKFramebuffer>& fbo)
        : _fbo(fbo) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _fbo->beginCommandBuffer(graphicsContext);
    }

private:
    sp<VKFramebuffer> _fbo;
};

class PostDrawElementsToFBO : public RenderCommand {
public:
    PostDrawElementsToFBO(const sp<VKFramebuffer>& fbo)
        : _fbo(fbo) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _fbo->endCommandBuffer(graphicsContext);
    }

private:
    sp<VKFramebuffer> _fbo;
};

}

VKFramebufferRenderer::VKFramebufferRenderer(sp<Renderer> delegate, sp<VKFramebuffer> framebuffer)
    : _delegate(std::move(delegate)), _fbo(std::move(framebuffer)), _pre_draw(sp<PreDrawElementsToFBO>::make(_fbo)), _post_draw(sp<PostDrawElementsToFBO>::make(_fbo))
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position)
{
    renderRequest.addRenderCommand(_pre_draw);
    _delegate->render(renderRequest, position);
    renderRequest.addRenderCommand(_post_draw);
}

}
}
