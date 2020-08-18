#ifndef ARK_RENDERER_VULKAN_BASE_VK_SUBMIT_QUEUE_H_
#define ARK_RENDERER_VULKAN_BASE_VK_SUBMIT_QUEUE_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKSubmitQueue {
public:
    VKSubmitQueue(const sp<VKRenderer>& renderer, VkPipelineStageFlags stageFlags, size_t numOfSignalSemaphores);
    ~VKSubmitQueue();

    const std::vector<VkSemaphore>& signalSemaphores() const;

    void begin(VkSemaphore waitSemaphore);

    void submitCommandBuffer(VkCommandBuffer commandBuffer);

    void submit(VkQueue queue);

    void addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers, uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores);
    void addWaitSemaphore(VkSemaphore semaphore);

private:
    VkSemaphore* aquireConnectorSemaphore();

private:
    sp<VKRenderer> _renderer;
    VkPipelineStageFlags _stage_flags;

    std::vector<VkCommandBuffer> _submit_queue;
    std::vector<VkSubmitInfo> _submit_infos;
    std::vector<VkSemaphore> _wait_semaphores;
    std::vector<VkSemaphore> _signal_semaphores;

    std::vector<VkSemaphore> _connector_semaphores;
    uint32_t _aquired_connector_index;
};

}
}

#endif
