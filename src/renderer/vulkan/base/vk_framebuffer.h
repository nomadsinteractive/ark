#ifndef ARK_RENDERER_VULKAN_BASE_VK_FRAMEBUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_FRAMEBUFFER_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKFramebuffer : public Resource {
public:
    VKFramebuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, const sp<Texture>& texture);
    ~VKFramebuffer() override;

    virtual uint64_t id() override;

    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    const sp<Texture>& texture() const;

    void beginCommandBuffer(GraphicsContext& graphicsContext);
    void endCommandBuffer(GraphicsContext& graphicsContext);
    void submit(GraphicsContext& graphicsContext);

private:
    sp<VKRenderer> _renderer;
    sp<Recycler> _recycler;
    sp<Texture> _texture;

    VkImage _depthstencil_image;
    VkDeviceMemory _depthstencil_memory;
    VkImageView _depthstencil_view;

    VkCommandBuffer _command_buffer;
    VkSemaphore _semaphore;

    VkDescriptorImageInfo _descriptor;
    VkClearValue _clear_values[2];

    VkCommandBufferBeginInfo _command_buffer_begin_info;
    VkRenderPassBeginInfo _render_pass_begin_info;
    VkViewport _viewport;
    VkRect2D _scissor;

    VkSubmitInfo _submit_info;
};

}
}

#endif
