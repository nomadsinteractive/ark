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
    : _renderer(renderer), _stage_flags(stageFlags), _signal_semaphores(numOfSignalSemaphores), _aquired_connector_index(0)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    for(VkSemaphore& i : _signal_semaphores)
        VKUtil::checkResult(vkCreateSemaphore(_renderer->vkLogicalDevice(), &semaphoreCreateInfo, nullptr, &i));
}

VKSubmitQueue::~VKSubmitQueue()
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();

    for(VkSemaphore& i : _signal_semaphores)
        vkDestroySemaphore(vkLogicalDevice, i, nullptr);

    for(VkSemaphore& i : _connector_semaphores)
        vkDestroySemaphore(vkLogicalDevice, i, nullptr);
}

const std::vector<VkSemaphore>& VKSubmitQueue::signalSemaphores() const
{
    return _signal_semaphores;
}

void VKSubmitQueue::begin(VkSemaphore waitSemaphore)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_infos.clear();
    _submit_queue.clear();
    _wait_semaphores.push_back(waitSemaphore);
    _aquired_connector_index = 0;
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
    submitInfo.pWaitDstStageMask = &_stage_flags;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = pCommandBuffers;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    if(!_submit_infos.empty())
    {
        VkSubmitInfo& preSubmitInfo = _submit_infos.back();
//        preSubmitInfo.pSignalSemaphores = aquireConnectorSemaphore();
//        preSubmitInfo.signalSemaphoreCount = 1;
//        submitInfo.pWaitSemaphores = preSubmitInfo.pSignalSemaphores;
//        submitInfo.waitSemaphoreCount = preSubmitInfo.signalSemaphoreCount;
    }
    _submit_infos.push_back(submitInfo);
}

void VKSubmitQueue::addWaitSemaphore(VkSemaphore semaphore)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _wait_semaphores.push_back(semaphore);
}

VkSemaphore* VKSubmitQueue::aquireConnectorSemaphore()
{
    DASSERT(_aquired_connector_index <= _connector_semaphores.size());
    if(_aquired_connector_index == _connector_semaphores.size())
    {
        VkSemaphore semaphore;
        VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
        VKUtil::checkResult(vkCreateSemaphore(_renderer->vkLogicalDevice(), &semaphoreCreateInfo, nullptr, &semaphore));
        _connector_semaphores.push_back(semaphore);
    }
    return &_connector_semaphores[_aquired_connector_index++];
}

}
}
