#include "vulkan/base/vk_submit_queue.h"

#include "graphics/base/color.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "vulkan/base/vk_semaphore.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

VKSubmitQueue::VKSubmitQueue(const sp<VKRenderer>& renderer, VkPipelineStageFlags stageFlags)
    : _renderer(renderer), _stage_flags{stageFlags}
{
}

sp<VKSemaphore> VKSubmitQueue::createSignalSemaphore()
{
    sp<VKSemaphore> semaphore = sp<VKSemaphore>::make(_renderer);
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
        std::vector<VkSemaphore> signalSemaphores;
        signalSemaphores.reserve(_signal_semaphores.size());
        for(auto iter = _signal_semaphores.begin(); iter != _signal_semaphores.end(); )
            if(iter->unique())
                iter = _signal_semaphores.erase(iter);
            else
            {
                signalSemaphores.push_back(iter->get()->vkSemaphore());
                ++iter;
            }

        VkSubmitInfo& firstSubmitInfo = _submit_infos.front();
        VkSubmitInfo& lastSubmitInfo = _submit_infos.back();
        firstSubmitInfo.pWaitSemaphores = _wait_semaphores.data();
        firstSubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(_wait_semaphores.size());
        lastSubmitInfo.pSignalSemaphores = signalSemaphores.data();
        lastSubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
        VKUtil::checkResult(vkQueueSubmit(queue, static_cast<uint32_t>(_submit_infos.size()), _submit_infos.data(), VK_NULL_HANDLE));
    }
    _wait_semaphores.clear();
}

void VKSubmitQueue::addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
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
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    DCHECK(_wait_semaphores.size() <= array_size(_stage_flags), "Too many stages");
    _wait_semaphores.push_back(semaphore);
    _stage_flags[_wait_semaphores.size()] = waitStageFlag;
}

}
