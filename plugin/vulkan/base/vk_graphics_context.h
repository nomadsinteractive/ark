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

    // Blocks until the GPU has finished the work previously submitted on the current frame-in-flight slot. Must be
    // called before RenderController::onDrawFrame writes this frame's (in-place, single-buffered) GPU resources, so
    // that they are not overwritten while still being read by the previous frame.
    void waitForFrameAvailable() const;
    // Image-available semaphore for the current frame slot; hand this to vkAcquireNextImageKHR.
    VkSemaphore currentPresentComplete() const;
    // Registers the just-acquired swapchain image with the current frame slot, waiting out any earlier in-flight
    // frame that still uses that image, and arms this frame's fence for the upcoming submit.
    void onImageAcquired(uint32_t imageId);
    void submit(VkQueue queue);
    // Render-complete semaphore for the current image, waited on by vkQueuePresentKHR.
    VkSemaphore currentRenderComplete() const;
    // Advances to the next frame-in-flight slot; call once the frame has been submitted and presented.
    void endFrame();

private:
    // Capped at 1 for now: per-frame GPU resources (uniform/vertex buffers updated via RenderController::onDrawFrame)
    // are single-buffered and written in place, so allowing more than one frame in flight would let the CPU
    // overwrite memory the GPU is still reading. Raising this to 2+ to actually overlap CPU/GPU work additionally
    // requires per-frame-in-flight ring-buffering of those dynamically-updated resources.
    static constexpr uint32_t kMaxFramesInFlight = 1;

    sp<VKRenderer> _renderer;
    sp<VKSwapChain> _render_target;

    sp<VKCommandBuffers> _command_buffers;

    VKSubmitQueue _submit_queue;

    uint32_t _image_count;
    uint32_t _current_frame;
    uint32_t _current_image;

    // Cycled per frame-in-flight: the image-available semaphore must be supplied to vkAcquireNextImageKHR
    // before the image index is known, so it cannot be indexed by swapchain image.
    VkSemaphore _present_complete[kMaxFramesInFlight];
    VkFence _in_flight_fences[kMaxFramesInFlight];

    // Indexed per swapchain image: vkQueuePresentKHR waits on the render-complete semaphore, and re-acquiring an
    // image guarantees its previous present has finished, making per-image reuse safe.
    Vector<VkSemaphore> _render_complete;
    // Non-owning: the in-flight fence currently guarding each swapchain image (VK_NULL_HANDLE if none).
    Vector<VkFence> _images_in_flight;

    Stack<State> _state_stack;
};

}
