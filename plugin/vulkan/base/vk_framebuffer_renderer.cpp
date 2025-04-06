#include "vulkan/base/vk_framebuffer_renderer.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_layer_snapshot.h"
#include "graphics/inf/render_command.h"

#include "renderer/impl/draw_decorator/draw_decorator_composite.h"
#include "renderer/inf/draw_decorator.h"

#include "vulkan/base/vk_framebuffer.h"

namespace ark::plugin::vulkan {

namespace {

class DrawDecoratorFBO final : public DrawDecorator {
public:
    DrawDecoratorFBO(sp<VKFramebuffer> fbo)
        : _fbo(std::move(fbo))
    {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        _fbo->beginRenderPass(graphicsContext);
    }

    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        const VkCommandBuffer commandBuffer = _fbo->endRenderPass(graphicsContext);
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
    }

private:
    sp<VKFramebuffer> _fbo;
};

}

VKFramebufferRenderer::VKFramebufferRenderer(sp<RenderLayer> renderLayer, sp<VKFramebuffer> framebuffer)
    : _render_layer(std::move(renderLayer)), _draw_decorator(sp<DrawDecorator>::make<DrawDecoratorFBO>(std::move(framebuffer)))
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    _render_layer->render(renderRequest, position, DrawDecoratorComposite::compose(drawDecorator, _draw_decorator));
}

}
