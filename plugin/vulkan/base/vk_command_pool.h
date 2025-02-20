#pragma once

#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKCommandPool {
public:
    VKCommandPool(const VKDevice& device, uint32_t queueFamilyIndex);
    ~VKCommandPool();

    VkQueue vkQueue() const;

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin) const;
    Vector<VkCommandBuffer> makeCommandBuffers(uint32_t count) const;

    void flushCommandBuffer(VkCommandBuffer commandBuffer, bool free) const;

    void destroyCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const;

private:
    VkDevice _logical_device;

    VkQueue _queue;
    VkCommandPool _command_pool;
};

}
