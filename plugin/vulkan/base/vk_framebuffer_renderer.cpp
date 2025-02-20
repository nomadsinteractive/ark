#include "vulkan/base/vk_framebuffer_renderer.h"

#include "graphics/base/render_request.h"
#include "graphics/inf/render_command.h"

#include "vulkan/base/vk_framebuffer.h"

namespace ark::plugin::vulkan {

namespace {

class PreDrawElementsToFBO : public RenderCommand {
public:
    PreDrawElementsToFBO(const sp<VKFramebuffer>& fbo)
        : _fbo(fbo)
    {
    }

    void draw(GraphicsContext& graphicsContext) override
    {
        _fbo->beginRenderPass(graphicsContext);
    }

private:
    sp<VKFramebuffer> _fbo;
};

class PostDrawElementsToFBO final : public RenderCommand {
public:
    PostDrawElementsToFBO(const sp<VKFramebuffer>& fbo)
        : _fbo(fbo)
    {
    }

    void draw(GraphicsContext& graphicsContext) override
    {
        const VkCommandBuffer commandBuffer = _fbo->endRenderPass(graphicsContext);
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
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
