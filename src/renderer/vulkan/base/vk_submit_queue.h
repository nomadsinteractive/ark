#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKSubmitQueue {
public:
    VKSubmitQueue(const sp<VKRenderer>& renderer, VkPipelineStageFlags stageFlags);
    ~VKSubmitQueue();

    VkSemaphore createSignalSemaphore();

    void begin(VkSemaphore waitSemaphore);
    void submitCommandBuffer(VkCommandBuffer commandBuffer);
    void submit(VkQueue queue);

    void addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
    void addWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitStageFlag);

private:
    VkSemaphore* aquireConnectorSemaphore();

private:
    sp<VKRenderer> _renderer;
    VkPipelineStageFlags _stage_flags[8];

    std::vector<VkCommandBuffer> _submit_queue;
    std::vector<VkSubmitInfo> _submit_infos;
    std::vector<VkSemaphore> _wait_semaphores;
    std::vector<VkSemaphore> _signal_semaphores;
};

}
