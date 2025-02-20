#pragma once

#include <list>
#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKSubmitQueue {
public:
    VKSubmitQueue(const sp<VKRenderer>& renderer, VkPipelineStageFlags stageFlags);

    sp<VKSemaphore> createSignalSemaphore();

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

    std::list<VkCommandBuffer> _submit_queue;
    std::vector<VkSubmitInfo> _submit_infos;
    std::vector<VkSemaphore> _wait_semaphores;

    std::vector<sp<VKSemaphore>> _signal_semaphores;
};

}
