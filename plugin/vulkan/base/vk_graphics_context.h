#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"
#include "vulkan/base/vk_submit_queue.h"

#include "platform/vulkan/vulkan.h"
#include "renderer/base/render_engine_context.h"

namespace ark::plugin::vulkan {

class VKGraphicsContext {
public:
    VKGraphicsContext(const GraphicsContext& graphicsContext, const sp<VKRenderer>& renderer);
    ~VKGraphicsContext();

    void begin(uint32_t imageId, const V4& backgroundColor);
    void end();

    VKSubmitQueue& submitQueue();

    class RenderPassPhrase {
    public:
        RenderPassPhrase(RenderEngineContext::Resolution resolution, uint32_t colorAttachmentCount, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
        virtual ~RenderPassPhrase() = default;

        const RenderEngineContext::Resolution& resolution() const;
        uint32_t colorAttachmentCount() const;
        VkCommandBuffer vkCommandBuffer() const;

        virtual Vector<VkPipelineColorBlendAttachmentState> makeColorBlendAttachmentStates(const VkPipelineColorBlendAttachmentState& mainState, uint32_t colorAttachmentCount) = 0;

        virtual VkRenderPass acquire() = 0;
        virtual VkRenderPass begin(VkCommandBuffer commandBuffer) = 0;

    protected:
        RenderEngineContext::Resolution _resolution;
        uint32_t _color_attachment_count;
        VkCommandBuffer _command_buffer;
    };

    class State {
    public:
        State(sp<RenderPassPhrase> renderPassPhrase, VkCommandBuffer commandBuffer, bool commandBufferBegan);

        VkRenderPass ensureRenderPass();
        VkCommandBuffer ensureRenderPassCommandBuffer();

    private:
        sp<RenderPassPhrase> _render_pass_phrase;
        VkCommandBuffer _command_buffer;
        bool _command_buffer_began;
        VkRenderPass _render_pass;

        friend class VKGraphicsContext;
        friend class VKPipeline;
    };

    State& currentState();

    void pushState(sp<RenderPassPhrase> renderPassPhrase);
    VkCommandBuffer popState();

    void submit(VkQueue queue);

    const sp<VKSemaphore>& semaphoreRenderComplete() const;
    VkSemaphore semaphorePresentComplete() const;

private:
    sp<VKRenderer> _renderer;
    sp<VKSwapChain> _render_target;

    sp<VKCommandBuffers> _command_buffers;

    VKSubmitQueue _submit_queue;
    sp<VKSemaphore> _semaphore_render_complete;
    VkSemaphore _semaphore_present_complete;

    Stack<State> _state_stack;
};

}
