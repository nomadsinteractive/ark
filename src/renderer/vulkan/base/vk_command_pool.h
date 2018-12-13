#ifndef ARK_RENDERER_VULKAN_BASE_VK_COMMAND_POOL_H_
#define ARK_RENDERER_VULKAN_BASE_VK_COMMAND_POOL_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKCommandPool {
public:
    VKCommandPool(const sp<VKDevice>& device, uint32_t queueNodeIndex);
    ~VKCommandPool();

    const sp<VKDevice>& device() const;

    VkCommandBuffer getCommandBuffer(bool begin) const;
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin) const;
    std::vector<VkCommandBuffer> makeCommandBuffers(uint32_t count) const;

    void flushCommandBuffer(VkCommandBuffer commandBuffer, bool free) const;

    void destroyCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const;

private:
    sp<VKDevice> _device;

    VkCommandPool _command_pool;

};

}
}

#endif
