#include "renderer/vulkan/base/vk_compute_context.h"

#include "graphics/base/color.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKComputeContext::VKComputeContext(GraphicsContext& graphicsContext, sp<VKRenderer> renderer)
    : _renderer(std::move(renderer)), _submit_infos(1), _command_buffer(VK_NULL_HANDLE)
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_semaphore_compute_complete));

    const sp<VKRenderTarget>& renderTarget = _renderer->renderTarget();
    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.attachments().ensure<VKGraphicsContext>();
    _command_buffers = sp<VKCommandBuffers>::make(graphicsContext.recycler(), renderTarget);
    VkSubmitInfo& submitInfo = _submit_infos[0];
    submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &_submit_pipeline_stages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_semaphore_compute_complete;

    _semaphore_render_complete = vkGraphicsContext->semaphoreRenderComplete();
}

VKComputeContext::~VKComputeContext()
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();
    vkDestroySemaphore(vkLogicalDevice, _semaphore_compute_complete, nullptr);
}

void VKComputeContext::begin(uint32_t imageId)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();

    _command_buffer = commandBuffers.at(imageId);

    VKUtil::checkResult(vkBeginCommandBuffer(_command_buffer, &cmdBufInfo));

    _submit_infos.resize(1);
    VkSubmitInfo& submitInfo = _submit_infos[0];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &_semaphore_render_complete;
}

void VKComputeContext::end()
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    VKUtil::checkResult(vkEndCommandBuffer(_command_buffer));
    _command_buffer = VK_NULL_HANDLE;
}

VkCommandBuffer VKComputeContext::start()
{
    if(_command_buffer == VK_NULL_HANDLE)
        begin(0);
    return _command_buffer;
}

VkCommandBuffer VKComputeContext::vkCommandBuffer() const
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    return _command_buffer;
}

VkSemaphore VKComputeContext::semaphoreComputeComplete() const
{
    return _semaphore_compute_complete;
}

}
}
