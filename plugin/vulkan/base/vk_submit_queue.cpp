#include "vulkan/base/vk_submit_queue.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "vulkan/base/vk_semaphore.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

VKSubmitQueue::VKSubmitQueue(const sp<VKRenderer>& renderer, const VkPipelineStageFlags stageFlags)
    : _renderer(renderer), _stage_flags{stageFlags}
{
}

sp<VKSemaphore> VKSubmitQueue::createSignalSemaphore()
{
    sp<VKSemaphore> semaphore = sp<VKSemaphore>::make(_renderer);
    _signal_semaphores.push_back(semaphore);
    return semaphore;
}

void VKSubmitQueue::begin(const VkSemaphore waitSemaphore)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    _submit_infos.clear();
    _submit_queue.clear();
    _wait_semaphores.push_back(waitSemaphore);
}

void VKSubmitQueue::submitCommandBuffer(VkCommandBuffer commandBuffer)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    _submit_queue.push_back(commandBuffer);
    addSubmitInfo(1, &_submit_queue.back());
}

void VKSubmitQueue::submit(const VkQueue queue, const VkSemaphore signalSemaphore, const VkFence fence)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);

    // Collect the persistently registered signal semaphores (e.g. the compute context chains off these),
    // dropping any that have since expired, then append the per-submit signal semaphore (the swapchain
    // render-complete semaphore) supplied by the caller.
    Vector<VkSemaphore> signalSemaphores;
    signalSemaphores.reserve(_signal_semaphores.size() + 1);
    for(auto iter = _signal_semaphores.begin(); iter != _signal_semaphores.end(); )
    {
        if(const sp<VKSemaphore> semaphore = iter->lock())
        {
            signalSemaphores.push_back(semaphore->vkSemaphore());
            ++iter;
        }
        else
            iter = _signal_semaphores.erase(iter);
    }
    if(signalSemaphore != VK_NULL_HANDLE)
        signalSemaphores.push_back(signalSemaphore);

    if(!_submit_infos.empty())
    {
        VkSubmitInfo& firstSubmitInfo = _submit_infos.front();
        VkSubmitInfo& lastSubmitInfo = _submit_infos.back();
        firstSubmitInfo.pWaitSemaphores = _wait_semaphores.data();
        firstSubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(_wait_semaphores.size());
        lastSubmitInfo.pSignalSemaphores = signalSemaphores.data();
        lastSubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
        VKUtil::checkResult(vkQueueSubmit(queue, static_cast<uint32_t>(_submit_infos.size()), _submit_infos.data(), fence));
    }
    else if(fence != VK_NULL_HANDLE || !_wait_semaphores.empty() || !signalSemaphores.empty())
    {
        // Degenerate frame that recorded no command buffers: still consume the wait semaphore(s), signal the
        // completion semaphore(s) and the fence so the frame-in-flight slot does not stall the next acquire.
        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.pWaitDstStageMask = _stage_flags;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(_wait_semaphores.size());
        submitInfo.pWaitSemaphores = _wait_semaphores.data();
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        VKUtil::checkResult(vkQueueSubmit(queue, 1, &submitInfo, fence));
    }
    _wait_semaphores.clear();
}

void VKSubmitQueue::addSubmitInfo(const uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
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
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    DCHECK(_wait_semaphores.size() <= array_size(_stage_flags), "Too many stages");
    _wait_semaphores.push_back(semaphore);
    _stage_flags[_wait_semaphores.size()] = waitStageFlag;
}

}
