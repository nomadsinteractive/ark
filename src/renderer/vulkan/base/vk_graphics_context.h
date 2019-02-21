#ifndef ARK_RENDERER_VULKAN_BASE_VK_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_VULKAN_BASE_VK_GRAPHICS_CONTEXT_H_

#include <stack>
#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKGraphicsContext {
public:
    VKGraphicsContext(const sp<VKRenderer>& renderer);
    ~VKGraphicsContext();

    void initialize(GraphicsContext& graphicsContext);

    void begin(uint32_t imageId, const Color& backgroundColor);
    void end();
    void submit(GraphicsContext& graphicsContext);

    VkCommandBuffer vkCommandBuffer() const;

    void pushCommandBuffer(VkCommandBuffer commandBuffer);
    VkCommandBuffer popCommandBuffer();

    void addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers, uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores);

    std::vector<VkSubmitInfo>& submitInfos();

private:
    sp<VKRenderer> _renderer;

    sp<VKCommandBuffers> _command_buffers;

    VkCommandBuffer _command_buffer;
    std::stack<VkCommandBuffer> _command_buffer_stack;

    std::vector<VkSubmitInfo> _submit_infos;

    VkPipelineStageFlags _submit_pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSemaphore _semaphore_present_complete;
    VkSemaphore _semaphore_render_complete;

    friend class VKRenderTarget;
};

}
}

#endif
