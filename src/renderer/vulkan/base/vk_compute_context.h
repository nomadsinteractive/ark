#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_submit_queue.h"
#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKComputeContext {
public:
    VKComputeContext(GraphicsContext& graphicsContext, sp<VKRenderer> renderer);

    void initialize(GraphicsContext& graphicsContext);

    void begin();
    void end();

    VkCommandBuffer buildCommandBuffer(GraphicsContext& graphicsContext);
    VkCommandBuffer vkCommandBuffer() const;

    sp<VKSemaphore> createCompleteSemaphore();

private:
    sp<VKRenderer> _renderer;
    sp<VKCommandPool> _command_pool;
    VKSubmitQueue _submit_queue;

    VkCommandBuffer _command_buffer;

    sp<VKSemaphore> _semaphore_render_complete;
};

}
