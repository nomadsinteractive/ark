#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/viewport.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"

#include "vulkan/util/vulkan_swap_chain.hpp"

namespace ark::plugin::vulkan {

class VKSwapChain {
public:
    VKSwapChain(const RenderEngine& renderEngine, sp<VKDevice> device);
    ~VKSwapChain();

    uint32_t width() const;
    uint32_t height() const;

    const VkRect2D& vkScissor() const;
    const VkViewport& vkViewport() const;

    const VkRenderPassBeginInfo& vkRenderPassBeginInfo() const;

    const sp<VKDevice>& device() const;

    const sp<VKCommandPool>& commandPool() const;
    const Vector<VkFramebuffer>& frameBuffers() const;

    Vector<VkCommandBuffer> makeCommandBuffers() const;

    uint32_t acquire(VKGraphicsContext& vkContext);
    uint32_t aquiredImageId() const;

    void swap(VKGraphicsContext& vkGraphicsContext);
    void waitIdle() const;

    void onSurfaceChanged(uint32_t width, uint32_t height);

private:
    void initialize(const RenderEngine& renderEngine);

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

    Vector<VkFramebuffer> _frame_buffers;

    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } _depth_stencil;

    uint32_t _width = 0;
    uint32_t _height = 0;

    uint32_t _aquired_image_id;
    bool _vsync;

    friend class VKGraphicsContext;
};

}
