#include "vulkan/base/vk_framebuffer_renderer.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/inf/render_command.h"

#include "vulkan/base/vk_framebuffer.h"

namespace ark::plugin::vulkan {

namespace {

class PreDrawElementsToFBO final : public RenderCommand {
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

VKFramebufferRenderer::VKFramebufferRenderer(sp<RenderLayer> renderLayer, sp<VKFramebuffer> framebuffer)
    : _render_layer(std::move(renderLayer)), _fbo(std::move(framebuffer)), _pre_draw(sp<RenderCommand>::make<PreDrawElementsToFBO>(_fbo)), _post_draw(sp<RenderCommand>::make<PostDrawElementsToFBO>(_fbo))
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position)
{
    renderRequest.addRenderCommand(_pre_draw);
    _render_layer->render(renderRequest, position);
    renderRequest.addRenderCommand(_post_draw);
}

}
