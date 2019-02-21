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
        _fbo->submit(graphicsContext);
    }

private:
    sp<VKFramebuffer> _fbo;
};

}

VKFramebufferRenderer::VKFramebufferRenderer(const sp<Renderer>& delegate, const sp<VKFramebuffer>& framebuffer)
    : _delegate(delegate), _fbo(framebuffer), _pre_draw(sp<PreDrawElementsToFBO>::make(_fbo)), _post_draw(sp<PostDrawElementsToFBO>::make(_fbo))
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addRequest(_pre_draw);
    _delegate->render(renderRequest, x, y);
    renderRequest.addRequest(_post_draw);
}

}
}
