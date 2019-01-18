#include "renderer/vulkan/render_view/render_view_vulkan.h"

#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/viewport.h"

#include "renderer/base/resource_manager.h"

#include "renderer/base/graphics_context.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VKRenderer>& renderer, const sp<RenderContext>& glContext, const sp<ResourceManager>& resourceManager, const Viewport& viewport)
    : _renderer(renderer), _graphics_context(new GraphicsContext(glContext, resourceManager)), _viewport(viewport)
{
}

void RenderViewVulkan::onSurfaceCreated()
{
}

void RenderViewVulkan::onSurfaceChanged(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d, Viewport (%.1f, %.1f, %.1f, %.1f)", width, height, _viewport.left(), _viewport.top(), _viewport.right(), _viewport.bottom());
    _graphics_context.reset(new GraphicsContext(_graphics_context->glContext(), _graphics_context->resourceManager()));

    _renderer->renderTarget()->onSurfaceChanged(width, height);
    _command_buffers = sp<VKCommandBuffers>::make(_graphics_context->resourceManager()->recycler(), _renderer->renderTarget());

    makeCommandBuffers(_graphics_context, Color::WHITE);
}

void RenderViewVulkan::onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand)
{
    const sp<VKRenderTarget>& renderTarget = _renderer->renderTarget();
    renderTarget->acquire();
    _graphics_context->onDrawFrame();

    if(backgroundColor != _background_color)
        makeCommandBuffers(_graphics_context, backgroundColor);

    _command_buffers->submit(_graphics_context);
    renderCommand->draw(_graphics_context);
    renderTarget->swap();
}

void RenderViewVulkan::makeCommandBuffers(GraphicsContext& /*graphicsContext*/, const Color& backgroundColor)
{
    VkClearValue vkClearValues[2];
    VkClearRect vkClearRect[2] = {{_renderer->renderTarget()->vkScissor(), 0, 1}, {_renderer->renderTarget()->vkScissor(), 0, 1}};
    vkClearValues[0].color = {{backgroundColor.red(), backgroundColor.green(), backgroundColor.blue(), backgroundColor.alpha()}};
    vkClearValues[1].depthStencil = {1.0f, 0};

    _background_color = backgroundColor;

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VkRenderPassBeginInfo renderPassBeginInfo = _renderer->renderTarget()->vkRenderPassBeginInfo();

    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();
    for (size_t i = 0; i < commandBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = _renderer->renderTarget()->frameBuffers()[i];
        VKUtil::checkResult(vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo));
        VkClearAttachment vkClearAttachments[2] = {{VK_IMAGE_ASPECT_COLOR_BIT, 0, vkClearValues[0]}, {VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT, 0, vkClearValues[1]}};
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdClearAttachments(commandBuffers[i], 2, vkClearAttachments, 2, vkClearRect);
        vkCmdSetViewport(commandBuffers[i], 0, 1, &_renderer->renderTarget()->vkViewport());
        vkCmdEndRenderPass(commandBuffers[i]);
        VKUtil::checkResult(vkEndCommandBuffer(commandBuffers[i]));
    }
}

}
}
