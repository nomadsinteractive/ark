#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_target.h"
#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_graphics_context.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKFramebuffer final : public Resource {
public:
    VKFramebuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, RenderTarget::CreateConfigure configure);
    ~VKFramebuffer() override;

    uint64_t id() override;

    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    void beginRenderPass(GraphicsContext& graphicsContext) const;
    VkCommandBuffer endRenderPass(GraphicsContext& graphicsContext) const;

private:
    class Stub final : public VKGraphicsContext::RenderPassPhrase {
    public:
        Stub(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, RenderTarget::CreateConfigure configure);

        void initialize();

        VkRenderPass acquire(const PipelineDescriptor& bindings) override;
        VkRenderPass begin(VkCommandBuffer commandBuffer) override;

    private:
        VkRect2D getFramebufferScissor() const;

    private:
        sp<VKRenderer> _renderer;
        sp<Recycler> _recycler;
        RenderTarget::CreateConfigure _configure;

        VkImage _depthstencil_image;
        VkDeviceMemory _depthstencil_memory;
        VkImageView _depthstencil_view;

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
