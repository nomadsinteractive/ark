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

VKGraphicsContext::VKGraphicsContext(GraphicsContext& graphicsContext, const sp<VKRenderer>& renderer)
    : _renderer(renderer), _render_target(_renderer->renderTarget()), _command_buffers(sp<VKCommandBuffers>::make(graphicsContext.recycler(), _render_target)),
      _submit_queue(_renderer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 2)
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();
    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_semaphore_present_complete));
}

VKGraphicsContext::~VKGraphicsContext()
{
    vkDestroySemaphore(_renderer->vkLogicalDevice(), _semaphore_present_complete, nullptr);
}

void VKGraphicsContext::begin(uint32_t imageId, const Color& backgroundColor)
{
    VkClearValue vkClearValues[2];
    vkClearValues[0].color = {{backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a()}};
    vkClearValues[1].depthStencil = {1.0f, 0};

    const VKRenderTarget& renderTarget = _renderer->renderTarget();

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VkRenderPassBeginInfo renderPassBeginInfo = renderTarget.vkRenderPassBeginInfo();

    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();

    VkCommandBuffer commandBuffer = commandBuffers.at(imageId);
    pushState({commandBuffer, renderPassBeginInfo.renderPass});
    renderPassBeginInfo.framebuffer = renderTarget.frameBuffers().at(imageId);
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = vkClearValues;
    VKUtil::checkResult(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &renderTarget.vkViewport());
    vkCmdSetScissor(commandBuffer, 0, 1, &renderTarget.vkScissor());

    _submit_queue.begin(_semaphore_present_complete);
}

void VKGraphicsContext::end()
{
    DASSERT(_state_stack.size() == 1);
    const State& state = _state_stack.top();
    vkCmdEndRenderPass(state._command_buffer);
    VKUtil::checkResult(vkEndCommandBuffer(state._command_buffer));
    _submit_queue.submitCommandBuffer(state._command_buffer);
    _state_stack.pop();
}

VkCommandBuffer VKGraphicsContext::vkCommandBuffer() const
{
    DASSERT(!_state_stack.empty());
    return _state_stack.top()._command_buffer;
}

VkRenderPass VKGraphicsContext::vkRenderPass() const
{
    DASSERT(!_state_stack.empty());
    return _state_stack.top()._render_pass;
}

void VKGraphicsContext::pushState(const State& state)
{
    _state_stack.push(state);
}

void VKGraphicsContext::popState()
{
    DASSERT(!_state_stack.empty());
    _state_stack.pop();
}

void VKGraphicsContext::submit(VkQueue queue)
{
    _submit_queue.submit(queue);
}

void VKGraphicsContext::addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    _submit_queue.addSubmitInfo(commandBufferCount, pCommandBuffers);
}

void VKGraphicsContext::addWaitSemaphore(VkSemaphore semaphore)
{
    _submit_queue.addWaitSemaphore(semaphore);
}

VkSemaphore VKGraphicsContext::semaphoreRenderComplete(size_t n) const
{
    return _submit_queue.signalSemaphores().at(n);
}

VkSemaphore VKGraphicsContext::semaphorePresentComplete() const
{
    return _semaphore_present_complete;
}

}
}
