#ifndef ARK_RENDERER_VULKAN_BASE_VK_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_VULKAN_BASE_VK_GRAPHICS_CONTEXT_H_

#include <stack>
#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_submit_queue.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKGraphicsContext {
public:
    VKGraphicsContext(GraphicsContext& graphicsContext, const sp<VKRenderer>& renderer);
    ~VKGraphicsContext();

    void begin(uint32_t imageId, const Color& backgroundColor);
    void end();

    struct State {
        VkCommandBuffer _command_buffer;
        VkRenderPass _render_pass;
    };

    VkCommandBuffer vkCommandBuffer() const;
    VkRenderPass vkRenderPass() const;

    void pushState(const State& state);
    void popState();

    void submit(VkQueue queue);

    void addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
    void addWaitSemaphore(VkSemaphore semaphore);

    VkSemaphore semaphoreRenderComplete(size_t n) const;
    VkSemaphore semaphorePresentComplete() const;

private:
    sp<VKRenderer> _renderer;
    sp<VKRenderTarget> _render_target;

    sp<VKCommandBuffers> _command_buffers;

    VKSubmitQueue _submit_queue;

    std::stack<State> _state_stack;

    VkSemaphore _semaphore_present_complete;
};

}
}

#endif
