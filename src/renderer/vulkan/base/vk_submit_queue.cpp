#include "renderer/vulkan/base/vk_submit_queue.h"

#include "graphics/base/color.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/util/vk_util.h"
#include "renderer/vulkan/util/vulkan_initializers.hpp"

namespace ark::vulkan {

VKSubmitQueue::VKSubmitQueue(const sp<VKRenderer>& renderer, VkPipelineStageFlags stageFlags)
    : _renderer(renderer), _stage_flags{stageFlags}
{
}

VKSubmitQueue::~VKSubmitQueue()
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();

    for(const VkSemaphore& i : _signal_semaphores)
        vkDestroySemaphore(vkLogicalDevice, i, nullptr);
}

VkSemaphore VKSubmitQueue::createSignalSemaphore()
{
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VKUtil::checkResult(vkCreateSemaphore(_renderer->vkLogicalDevice(), &semaphoreCreateInfo, nullptr, &semaphore));
    _signal_semaphores.push_back(semaphore);
    return semaphore;
}

void VKSubmitQueue::begin(VkSemaphore waitSemaphore)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_infos.clear();
    _submit_queue.clear();
    _wait_semaphores.push_back(waitSemaphore);
}

void VKSubmitQueue::submitCommandBuffer(VkCommandBuffer commandBuffer)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_queue.push_back(commandBuffer);
    addSubmitInfo(1, &_submit_queue.back());
}

void VKSubmitQueue::submit(VkQueue queue)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    if(!_submit_infos.empty())
    {
        VkSubmitInfo& firstSubmitInfo = _submit_infos.front();
        VkSubmitInfo& lastSubmitInfo = _submit_infos.back();
        firstSubmitInfo.pWaitSemaphores = _wait_semaphores.data();
        firstSubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(_wait_semaphores.size());
        lastSubmitInfo.pSignalSemaphores = _signal_semaphores.data();
        lastSubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(_signal_semaphores.size());
        VKUtil::checkResult(vkQueueSubmit(queue, static_cast<uint32_t>(_submit_infos.size()), _submit_infos.data(), VK_NULL_HANDLE));
    }
    _wait_semaphores.clear();
}

void VKSubmitQueue::addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = _stage_flags;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = pCommandBuffers;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    _submit_infos.push_back(submitInfo);
}

void VKSubmitQueue::addWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitStageFlag)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    DCHECK(_wait_semaphores.size() <= array_size(_stage_flags), "Too many stages");
    _wait_semaphores.push_back(semaphore);
    _stage_flags[_wait_semaphores.size()] = waitStageFlag;
}

}
