#include "renderer/vulkan/base/vk_graphics_context.h"

#include "graphics/base/color.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKGraphicsContext::VKGraphicsContext(const sp<VKRenderer>& renderer)
    : _renderer(renderer), _submit_infos(1)
{
    VkDevice vkLogicalDevice = renderer->vkLogicalDevice();
    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_semaphore_present_complete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_semaphore_render_complete));
}

VKGraphicsContext::~VKGraphicsContext()
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();
    vkDestroySemaphore(vkLogicalDevice, _semaphore_present_complete, nullptr);
    vkDestroySemaphore(vkLogicalDevice, _semaphore_render_complete, nullptr);
}

void VKGraphicsContext::initialize(GraphicsContext& graphicsContext)
{
    const sp<VKRenderTarget>& renderTarget = _renderer->renderTarget();
    _command_buffers = sp<VKCommandBuffers>::make(graphicsContext.recycler(), renderTarget);
    VkSubmitInfo& submitInfo = _submit_infos[0];
    submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &_submit_pipeline_stages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_semaphore_render_complete;
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

    _submit_infos.resize(1);
    VkSubmitInfo& submitInfo = _submit_infos[0];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &_semaphore_present_complete;
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

void VKGraphicsContext::pushCommandBuffer(VkCommandBuffer commandBuffer)
{
    _command_buffer_stack.push(_command_buffer);
    _command_buffer = commandBuffer;
}

VkCommandBuffer VKGraphicsContext::popCommandBuffer()
{
    DASSERT(!_command_buffer_stack.empty());
    _command_buffer = _command_buffer_stack.top();
    _command_buffer_stack.pop();
    return _command_buffer;
}

void VKGraphicsContext::addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers, uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores)
{
    VkSubmitInfo& preSubmitInfo = _submit_infos.back();
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &_submit_pipeline_stages;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = pCommandBuffers;
    submitInfo.signalSemaphoreCount = signalSemaphoreCount;
    submitInfo.pSignalSemaphores = pSignalSemaphores;
    submitInfo.pWaitSemaphores = preSubmitInfo.pWaitSemaphores;
    preSubmitInfo.pWaitSemaphores = pSignalSemaphores;
    _submit_infos.push_back(submitInfo);
}

std::vector<VkSubmitInfo>& VKGraphicsContext::submitInfos()
{
    return _submit_infos;
}

}
}
