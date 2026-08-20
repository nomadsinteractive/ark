#include "vulkan/base/vk_framebuffer_renderer.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_layer_snapshot.h"
#include "graphics/inf/render_command.h"
#include "renderer/base/graphics_context.h"

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
        const VKGraphicsContext::State state = graphicsContext.traits().ensure<VKGraphicsContext>()->popState();
        constexpr VkMemoryBarrier memoryBarrier = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
        };
        if(state._command_buffer_began)
            vkCmdPipelineBarrier(state._command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
    }

private:
    sp<VKFramebuffer> _fbo;
};

}

VKFramebufferRenderer::VKFramebufferRenderer(sp<Renderer> renderer, sp<VKFramebuffer> framebuffer)
    : _renderer(std::move(renderer)), _draw_decorator(sp<DrawDecorator>::make<DrawDecoratorFBO>(std::move(framebuffer)))
{
}

void VKFramebufferRenderer::render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator)
{
    _renderer->render(renderRequest, position, DrawDecoratorComposite::compose(drawDecorator, _draw_decorator));
}

}
