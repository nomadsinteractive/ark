#pragma once


#include "core/types/weak_ptr.h"

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

    List<VkCommandBuffer> _submit_queue;
    Vector<VkSubmitInfo> _submit_infos;
    Vector<VkSemaphore> _wait_semaphores;

    Vector<WeakPtr<VKSemaphore>> _signal_semaphores;
};

}
