#include "renderer/vulkan/base/vk_submit_queue.h"

#include "graphics/base/color.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/util/vk_util.h"
#include "renderer/vulkan/util/vulkan_initializers.hpp"

namespace ark {
namespace vulkan {

VKSubmitQueue::VKSubmitQueue(const sp<VKRenderer>& renderer, VkPipelineStageFlags stageFlags, size_t numOfSignalSemaphores)
    : _renderer(renderer), _stage_flags(stageFlags), _submit_infos(1), _signal_semaphores(numOfSignalSemaphores)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();

    for(VkSemaphore& i : _signal_semaphores)
        VKUtil::checkResult(vkCreateSemaphore(_renderer->vkLogicalDevice(), &semaphoreCreateInfo, nullptr, &i));

    VkSubmitInfo& submitInfo = _submit_infos[0];
    submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &_stage_flags;
    submitInfo.signalSemaphoreCount = _signal_semaphores.size();
    submitInfo.pSignalSemaphores = _signal_semaphores.data();
}

VKSubmitQueue::~VKSubmitQueue()
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();

    for(VkSemaphore& i : _signal_semaphores)
        vkDestroySemaphore(vkLogicalDevice, i, nullptr);
}

const std::vector<VkSemaphore>& VKSubmitQueue::signalSemaphores() const
{
    return _signal_semaphores;
}

void VKSubmitQueue::begin(VkSemaphore waitSemaphore)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_infos.resize(1);
    _wait_semaphores.push_back(waitSemaphore);
    _submit_queue.clear();
}

void VKSubmitQueue::submitCommandBuffer(VkCommandBuffer commandBuffer)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_queue.push_back(commandBuffer);
}

void VKSubmitQueue::submit(VkQueue queue)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    VkSubmitInfo& submitInfo = _submit_infos[0];
    submitInfo.pCommandBuffers = _submit_queue.data();
    submitInfo.commandBufferCount = static_cast<uint32_t>(_submit_queue.size());
    submitInfo.pWaitSemaphores = _wait_semaphores.data();
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(_wait_semaphores.size());

    VKUtil::checkResult(vkQueueSubmit(queue, static_cast<uint32_t>(_submit_infos.size()), _submit_infos.data(), VK_NULL_HANDLE));

    _wait_semaphores.clear();
}

void VKSubmitQueue::addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers, uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &_stage_flags;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = pCommandBuffers;
    submitInfo.signalSemaphoreCount = signalSemaphoreCount;
    submitInfo.pSignalSemaphores = pSignalSemaphores;

    VkSubmitInfo& preSubmitInfo = _submit_infos.back();
    submitInfo.pWaitSemaphores = preSubmitInfo.pWaitSemaphores;
    preSubmitInfo.pWaitSemaphores = pSignalSemaphores;

    _submit_infos.push_back(submitInfo);
}

void VKSubmitQueue::addWaitSemaphore(VkSemaphore semaphore)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _wait_semaphores.push_back(semaphore);
}

}
}
