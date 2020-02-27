#ifndef ARK_RENDERER_VULKAN_BASE_VK_RENDER_TARGET_H_
#define ARK_RENDERER_VULKAN_BASE_VK_RENDER_TARGET_H_

#include <vector>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/viewport.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "renderer/vulkan/util/vulkan_swap_chain.hpp"

namespace ark {
namespace vulkan {

class VKRenderTarget {
public:
    VKRenderTarget(const RenderEngineContext& renderContext, sp<VKDevice>& device);
    ~VKRenderTarget();

    uint32_t width() const;
    uint32_t height() const;

    const VkRect2D& vkScissor() const;
    const VkViewport& vkViewport() const;

    const VkRenderPassBeginInfo& vkRenderPassBeginInfo() const;

    const sp<VKDevice>& device() const;

    const sp<VKCommandPool>& commandPool() const;
    VkRenderPass vkRenderPass() const;
    const std::vector<VkFramebuffer>& frameBuffers() const;

    std::vector<VkCommandBuffer> makeCommandBuffers() const;
    sp<VKDescriptorPool> makeDescriptorPool(const sp<Recycler>& recycler) const;

    uint32_t acquire(VKGraphicsContext& vkContext);
    uint32_t aquiredImageId() const;

    void submit(VkCommandBuffer commandBuffer);
    void swap(VKGraphicsContext& vkContext);

    void onSurfaceChanged(uint32_t width, uint32_t height);

private:
    void initSwapchain(const RenderEngineContext& renderContext);
    void createCommandPool();
    void setupDepthStencil();
    void setupRenderPass();
    void setupFrameBuffer();

private:
    sp<VKDevice> _device;
    sp<VKCommandPool> _command_pool;

    VkQueue _queue;
    VulkanSwapChain _swap_chain;

    VkClearValue _clear_values[2];
    VkRenderPassBeginInfo _render_pass_begin_info;

    VkRect2D _scissor;
    VkViewport _viewport;

    std::vector<VkFramebuffer> _frame_buffers;
    std::vector<VkCommandBuffer> _submit_queue;

    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } _depth_stencil;

    uint32_t _width = 0;
    uint32_t _height = 0;

    uint32_t _aquired_image_id;

    friend class VKGraphicsContext;
};

}
}
#endif
