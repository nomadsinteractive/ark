#ifndef ARK_RENDERER_VULKAN_BASE_VK_RENDER_TARGET_H_
#define ARK_RENDERER_VULKAN_BASE_VK_RENDER_TARGET_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"

#include "renderer/vulkan/util/vulkan_swap_chain.hpp"

namespace ark {
namespace vulkan {

class VKRenderTarget {
public:
    VKRenderTarget(const sp<VKDevice>& device);
    ~VKRenderTarget();

    uint32_t width() const;
    uint32_t height() const;

    VkDescriptorPool descriptorPool() const;

    const sp<VKDevice>& device() const;

    const sp<VKCommandPool>& commandPool() const;
    VkRenderPass vkRenderPass() const;
    const std::vector<VkFramebuffer>& frameBuffers() const;

    std::vector<VkCommandBuffer> makeCommandBuffers() const;

    uint32_t acquire() const;
    void submit(VkCommandBuffer* commandBuffer);
    void swap(uint32_t currentBuffer);

private:
    void initSwapchain();
    void createCommandPool();
    void setupDepthStencil();
    void setupRenderPass();
    void setupFrameBuffer();

    void setupDescriptorPool();

private:
    sp<VKDevice> _device;
    sp<VKCommandPool> _command_pool;

    VkQueue _queue;
    VulkanSwapChain _swap_chain;
    VkRenderPass _render_pass;

    std::vector<VkFramebuffer> _frame_buffers;

    VkSemaphore _semaphore_present_complete;
    VkSemaphore _semaphore_render_complete;

    VkSubmitInfo _submit_info;
    VkPipelineStageFlags _submit_pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkDescriptorPool _descriptor_pool;

    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } _depth_stencil;

    uint32_t _width = 1280;
    uint32_t _height = 720;
};

}
}
#endif
