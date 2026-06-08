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

    // Ends and submits a command buffer WITHOUT waiting for completion and without freeing it. The caller must keep
    // the command buffer alive until the GPU is done with it (e.g. via the recycler's deferred destruction), since a
    // pending command buffer cannot be freed. GPU-side ordering must be arranged with a pipeline barrier/semaphore.
    void submitCommandBuffer(VkCommandBuffer commandBuffer) const;

    void destroyCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const;

private:
    VkDevice _logical_device;

    VkQueue _queue;
    VkCommandPool _command_pool;
};

}
