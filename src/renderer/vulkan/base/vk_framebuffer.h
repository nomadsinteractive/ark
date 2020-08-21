#ifndef ARK_RENDERER_VULKAN_BASE_VK_FRAMEBUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_FRAMEBUFFER_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_graphics_context.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKFramebuffer : public Resource {
public:
    VKFramebuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask);
    ~VKFramebuffer() override;

    virtual uint64_t id() override;

    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    void beginCommandBuffer(GraphicsContext& graphicsContext);
    void endCommandBuffer(GraphicsContext& graphicsContext);

private:
    class Stub : public VKGraphicsContext::RenderPassPhrase {
    public:
        Stub(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask);

        void initialize();

        virtual VkCommandBuffer vkCommandBuffer() override;
        virtual VkRenderPass create(const PipelineBindings& bindings) override;
        virtual VkRenderPass begin(VkCommandBuffer commandBuffer) override;

    private:
        VkRect2D getFramebufferScissor() const;

    private:
        sp<VKRenderer> _renderer;
        sp<Recycler> _recycler;
        std::vector<sp<Texture>> _color_attachments;
        sp<Texture> _depth_stencil_attachment;

        VkImage _depthstencil_image;
        VkDeviceMemory _depthstencil_memory;
        VkImageView _depthstencil_view;

        VkCommandBuffer _command_buffer;
        VkRenderPassBeginInfo _render_pass_begin_info;
        VkRect2D _scissor;
        VkViewport _viewport;

        std::vector<VkClearValue> _clear_values;

        friend class VKFramebuffer;
    };

private:
    sp<Stub> _stub;
};

}
}

#endif
