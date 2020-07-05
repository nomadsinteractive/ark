#ifndef ARK_RENDERER_VULKAN_BASE_VK_COMPUTE_CONTEXT_H_
#define ARK_RENDERER_VULKAN_BASE_VK_COMPUTE_CONTEXT_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKComputeContext {
public:
    VKComputeContext(GraphicsContext& graphicsContext, sp<VKRenderer> renderer);
    ~VKComputeContext();

    void initialize(GraphicsContext& graphicsContext);

    void begin(uint32_t imageId);
    void end();

    VkCommandBuffer start();
    VkCommandBuffer vkCommandBuffer() const;

    VkSemaphore semaphoreComputeComplete() const;

private:
    sp<VKRenderer> _renderer;
    sp<VKCommandBuffers> _command_buffers;

    std::vector<VkSubmitInfo> _submit_infos;

    VkPipelineStageFlags _submit_pipeline_stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    VkCommandBuffer _command_buffer;

    VkSemaphore _semaphore_render_complete;
    VkSemaphore _semaphore_compute_complete;
};

}
}

#endif
