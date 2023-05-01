#ifndef ARK_RENDERER_VULKAN_BASE_VK_COMPUTE_CONTEXT_H_
#define ARK_RENDERER_VULKAN_BASE_VK_COMPUTE_CONTEXT_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_submit_queue.h"
#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKComputeContext {
public:
    VKComputeContext(GraphicsContext& graphicsContext, sp<VKRenderer> renderer);

    void initialize(GraphicsContext& graphicsContext);

    void begin();
    void end();
    void submit();

    VkCommandBuffer start(GraphicsContext& graphicsContext);
    VkCommandBuffer vkCommandBuffer() const;

    VkSemaphore semaphoreComputeComplete() const;

private:
    sp<VKRenderer> _renderer;
    sp<VKCommandPool> _command_pool;
    VKSubmitQueue _submit_queue;

    VkCommandBuffer _command_buffer;

    VkSemaphore _semaphore_render_complete;
    VkSemaphore _semaphore_compute_complete;
};

}
}

#endif
