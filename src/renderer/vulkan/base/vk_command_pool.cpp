#include "renderer/vulkan/base/vk_command_pool.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKCommandPool::VKCommandPool(const VKDevice& device, uint32_t queueFamilyIndex)
    : _logical_device(device.vkLogicalDevice()), _queue(device.getQueueByFamilyIndex(queueFamilyIndex))
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VKUtil::checkResult(vkCreateCommandPool(_logical_device, &cmdPoolInfo, nullptr, &_command_pool));
}

VKCommandPool::~VKCommandPool()
{
    vkDestroyCommandPool(_logical_device, _command_pool, nullptr);
}

VkQueue VKCommandPool::vkQueue() const
{
    return _queue;
}

VkCommandBuffer VKCommandPool::createCommandBuffer(VkCommandBufferLevel level, bool begin) const
{
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            vks::initializers::commandBufferAllocateInfo(
                _command_pool,
                level,
                1);

    VKUtil::checkResult(vkAllocateCommandBuffers(_logical_device, &cmdBufAllocateInfo, &cmdBuffer));

    if(begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
        VKUtil::checkResult(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }

    return cmdBuffer;
}

std::vector<VkCommandBuffer> VKCommandPool::makeCommandBuffers(uint32_t count) const
{
    std::vector<VkCommandBuffer> commandBuffers(count);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            vks::initializers::commandBufferAllocateInfo(
                _command_pool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                count);

    VKUtil::checkResult(vkAllocateCommandBuffers(_logical_device, &cmdBufAllocateInfo, commandBuffers.data()));
    return commandBuffers;
}

void VKCommandPool::flushCommandBuffer(VkCommandBuffer commandBuffer, bool free) const
{
    if(commandBuffer == VK_NULL_HANDLE)
        return;

    VKUtil::checkResult(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VKUtil::checkResult(vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE));
    VKUtil::checkResult(vkQueueWaitIdle(_queue));

    if(free)
        vkFreeCommandBuffers(_logical_device, _command_pool, 1, &commandBuffer);
}

void VKCommandPool::destroyCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const
{
    vkFreeCommandBuffers(_logical_device, _command_pool, commandBufferCount, pCommandBuffers);
}

}
}
