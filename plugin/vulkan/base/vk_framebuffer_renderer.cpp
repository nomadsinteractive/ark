#include "vulkan/base/vk_framebuffer_renderer.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_layer_snapshot.h"
#include "graphics/base/render_request.h"
#include "graphics/inf/render_command.h"

#include "vulkan/base/vk_framebuffer.h"

namespace ark::plugin::vulkan {

namespace {

class RenderCommandFBO final : public RenderCommand {
public:
    RenderCommandFBO(const sp<VKFramebuffer>& fbo, sp<RenderCommand> delegate)
        : _fbo(fbo), _delegate(std::move(delegate))
    {
    }

    void draw(GraphicsContext& graphicsContext) override
    {
        _fbo->beginRenderPass(graphicsContext);
        _delegate->draw(graphicsContext);
        const VkCommandBuffer commandBuffer = _fbo->endRenderPass(graphicsContext);
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
    }

private:
    sp<VKFramebuffer> _fbo;
    sp<RenderCommand> _delegate;
};

}

VKFramebufferRenderer::VKFramebufferRenderer(sp<RenderLayer> renderLayer, sp<VKFramebuffer> framebuffer)
    : _render_layer(std::move(renderLayer)), _framebuffer(std::move(framebuffer))
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position)
{
    sp<RenderCommand> renderCommand = _render_layer->snapshot(renderRequest).compose(renderRequest);
    renderRequest.addRenderCommand(sp<RenderCommand>::make<RenderCommandFBO>(_framebuffer, std::move(renderCommand)));
}

}
