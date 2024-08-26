#pragma once

#include <stack>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_submit_queue.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKGraphicsContext {
public:
    VKGraphicsContext(GraphicsContext& graphicsContext, const sp<VKRenderer>& renderer);
    ~VKGraphicsContext();

    void begin(uint32_t imageId, const Color& backgroundColor);
    void end();

    VKSubmitQueue& submitQueue();

    class RenderPassPhrase {
    public:
        virtual ~RenderPassPhrase() = default;

        virtual VkCommandBuffer vkCommandBuffer() = 0;
        virtual VkRenderPass acquire(const PipelineDescriptor& bindings) = 0;
        virtual VkRenderPass begin(VkCommandBuffer commandBuffer) = 0;
    };

    class State {
    public:
        State(sp<RenderPassPhrase> renderPassPhrase, VkCommandBuffer commandBuffer, bool beginCommandBuffer);

        VkRenderPass acquireRenderPass(const PipelineDescriptor& bindings);
        VkCommandBuffer startRecording();

    private:
        sp<RenderPassPhrase> _render_pass_phrase;

        VkCommandBuffer _command_buffer;
        bool _begin_command_buffer;
        VkRenderPass _render_pass;

        friend class VKGraphicsContext;
    };

    State& getCurrentState();

    void pushState(sp<RenderPassPhrase> starter);
    void popState();

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

    std::stack<State> _state_stack;
};

}
