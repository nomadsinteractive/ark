#include "renderer/vulkan/base/vk_graphics_context.h"

#include "graphics/base/color.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_manager.h"

#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKGraphicsContext::VKGraphicsContext(const sp<VKRenderer>& renderer)
    : _renderer(renderer)
{
}

void VKGraphicsContext::initialize(GraphicsContext& graphicsContext)
{
    _command_buffers = sp<VKCommandBuffers>::make(graphicsContext.resourceManager()->recycler(), _renderer->renderTarget());
}

void VKGraphicsContext::begin(uint32_t imageId, const Color& backgroundColor)
{
    VkClearValue vkClearValues[2];
    vkClearValues[0].color = {{backgroundColor.red(), backgroundColor.green(), backgroundColor.blue(), backgroundColor.alpha()}};
    vkClearValues[1].depthStencil = {1.0f, 0};

    const VKRenderTarget& renderTarget = _renderer->renderTarget();

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VkRenderPassBeginInfo renderPassBeginInfo = renderTarget.vkRenderPassBeginInfo();

    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();

    _command_buffer = commandBuffers.at(imageId);

    renderPassBeginInfo.framebuffer = renderTarget.frameBuffers().at(imageId);
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = vkClearValues;
    VKUtil::checkResult(vkBeginCommandBuffer(_command_buffer, &cmdBufInfo));
    vkCmdBeginRenderPass(_command_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(_command_buffer, 0, 1, &renderTarget.vkViewport());
    vkCmdSetScissor(_command_buffer, 0, 1, &renderTarget.vkScissor());
}

void VKGraphicsContext::end()
{
    vkCmdEndRenderPass(_command_buffer);
    VKUtil::checkResult(vkEndCommandBuffer(_command_buffer));
}

void VKGraphicsContext::submit(GraphicsContext& graphicsContext)
{
    _command_buffers->submit(graphicsContext);
}

VkCommandBuffer VKGraphicsContext::vkCommandBuffer() const
{
    return _command_buffer;
}



}
}
