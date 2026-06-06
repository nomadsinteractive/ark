#include "vulkan/base/vk_graphics_context.h"

#include "graphics/inf/renderer.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "vulkan/base/vk_command_buffers.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/base/vk_swap_chain.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

namespace {

VkClearColorValue toVkClearColorValue(const V4& rgba)
{
    return {{rgba.x(), rgba.y(), rgba.z(), rgba.w()}};
}

class MainRenderPassPhrase final : public VKGraphicsContext::RenderPassPhrase {
public:
    MainRenderPassPhrase(const RenderBackendInfo::Resolution& resolution, const sp<VKRenderer>& renderer, const VkCommandBuffer commandBuffer, const VkFramebuffer framebuffer, const V4& backgroundColor)
        : RenderPassPhrase(resolution, 1, commandBuffer), _renderer(renderer), _framebuffer(framebuffer), _clear_color_value(toVkClearColorValue(backgroundColor)) {
    }

    Vector<VkPipelineColorBlendAttachmentState> makeColorBlendAttachmentStates(const VkPipelineColorBlendAttachmentState& mainState, const uint32_t colorAttachmentCount) override
    {
        ASSERT(colorAttachmentCount == 1);
        return {mainState};
    }

    VkRenderPass acquire() override
    {
        const VKSwapChain& renderTarget = _renderer->renderTarget();
        return renderTarget.vkRenderPassBeginInfo().renderPass;
    }

    VkRenderPass begin(const VkCommandBuffer commandBuffer) override
    {
        VkClearValue vkClearValues[2];
        vkClearValues[0].color = _clear_color_value;
        vkClearValues[1].depthStencil = {1.0f, 0};

        const VKSwapChain& renderTarget = _renderer->renderTarget();
        VkRenderPassBeginInfo renderPassBeginInfo = renderTarget.vkRenderPassBeginInfo();

        renderPassBeginInfo.framebuffer = _framebuffer;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = vkClearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(commandBuffer, 0, 1, &renderTarget.vkViewport());
        vkCmdSetScissor(commandBuffer, 0, 1, &renderTarget.vkScissor());
        return renderPassBeginInfo.renderPass;
    }

private:
    sp<VKRenderer> _renderer;
    VkFramebuffer _framebuffer;
    VkClearColorValue _clear_color_value;
};

}

VKGraphicsContext::VKGraphicsContext(const GraphicsContext& graphicsContext, const sp<VKRenderer>& renderer)
    : _renderer(renderer), _render_target(_renderer->renderTarget()), _command_buffers(sp<VKCommandBuffers>::make(graphicsContext.recycler(), _render_target)),
      _submit_queue(_renderer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT), _image_count(static_cast<uint32_t>(_command_buffers->vkCommandBuffers().size())),
      _current_frame(0), _current_image(0)
{
    const VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();

    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    // Fences start signaled so that the first vkWaitForFences on each frame slot returns immediately.
    constexpr VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };

    // Per frame-in-flight image-available semaphores and submit fences.
    for(uint32_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_present_complete[i]));
        VKUtil::checkResult(vkCreateFence(vkLogicalDevice, &fenceCreateInfo, nullptr, &_in_flight_fences[i]));
    }

    // Per swapchain image render-complete semaphores.
    _render_complete.resize(_image_count);
    for(uint32_t i = 0; i < _image_count; ++i)
        VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_render_complete[i]));

    _images_in_flight.resize(_image_count, VK_NULL_HANDLE);
}

VKGraphicsContext::~VKGraphicsContext()
{
    const VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();
    // The synchronization objects may still be referenced by in-flight GPU work (this context can be torn down
    // mid-pipeline on shutdown or surface resize), so drain the device before destroying them.
    vkDeviceWaitIdle(vkLogicalDevice);

    for(uint32_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        vkDestroySemaphore(vkLogicalDevice, _present_complete[i], nullptr);
        vkDestroyFence(vkLogicalDevice, _in_flight_fences[i], nullptr);
    }
    for(const VkSemaphore renderComplete : _render_complete)
        vkDestroySemaphore(vkLogicalDevice, renderComplete, nullptr);
}

void VKGraphicsContext::begin(const uint32_t imageId, const V4& backgroundColor)
{
    const VKSwapChain& renderTarget = _renderer->renderTarget();
    const Vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();

    VkCommandBuffer commandBuffer = commandBuffers.at(imageId);
    const RenderBackendInfo::Resolution resolution = {_render_target->width(), _render_target->height()};
    _state_stack.emplace(sp<RenderPassPhrase>::make<MainRenderPassPhrase>(resolution, _renderer, commandBuffer, renderTarget.frameBuffers().at(imageId), backgroundColor), commandBuffer, true);

    constexpr VkCommandBufferBeginInfo cmdBufInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VKUtil::checkResult(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

    _submit_queue.begin(_present_complete[_current_frame]);
}

void VKGraphicsContext::end()
{
    DASSERT(_state_stack.size() == 1);
    popState();
}

VKSubmitQueue& VKGraphicsContext::submitQueue()
{
    return _submit_queue;
}

VKGraphicsContext::RenderPassPhrase::RenderPassPhrase(const RenderBackendInfo::Resolution resolution, const uint32_t colorAttachmentCount, const VkCommandBuffer commandBuffer)
    : _resolution(resolution), _color_attachment_count(colorAttachmentCount), _command_buffer(commandBuffer)
{
}

const RenderBackendInfo::Resolution& VKGraphicsContext::RenderPassPhrase::resolution() const
{
    return _resolution;
}

uint32_t VKGraphicsContext::RenderPassPhrase::colorAttachmentCount() const
{
    return _color_attachment_count;
}

VkCommandBuffer VKGraphicsContext::RenderPassPhrase::vkCommandBuffer() const
{
    return _command_buffer;
}

VKGraphicsContext::State& VKGraphicsContext::currentState()
{
    ASSERT(!_state_stack.empty());
    return _state_stack.top();
}

void VKGraphicsContext::pushState(sp<RenderPassPhrase> renderPassPhrase)
{
    DCHECK(!_state_stack.empty(), "First state must be push by VKGraphicsContext::begin");
    const bool commandBufferBegan = _state_stack.top()._render_pass == VK_NULL_HANDLE;
    const VkCommandBuffer commandBuffer = commandBufferBegan ? _state_stack.top()._command_buffer : renderPassPhrase->vkCommandBuffer();
    _state_stack.push(State(std::move(renderPassPhrase), commandBuffer, commandBufferBegan));
}

VkCommandBuffer VKGraphicsContext::popState()
{
    DASSERT(!_state_stack.empty());
    const State state = _state_stack.top();
    _state_stack.pop();

    const VkCommandBuffer commandBuffer = state._command_buffer;
    if(state._render_pass != VK_NULL_HANDLE)
        vkCmdEndRenderPass(commandBuffer);

    if(_state_stack.empty() || _state_stack.top()._command_buffer != commandBuffer)
    {
        VKUtil::checkResult(vkEndCommandBuffer(commandBuffer));
        _submit_queue.submitCommandBuffer(commandBuffer);
    }

    return commandBuffer;
}

void VKGraphicsContext::waitForFrameAvailable() const
{
    VKUtil::checkResult(vkWaitForFences(_renderer->vkLogicalDevice(), 1, &_in_flight_fences[_current_frame], VK_TRUE, UINT64_MAX));
}

VkSemaphore VKGraphicsContext::currentPresentComplete() const
{
    return _present_complete[_current_frame];
}

void VKGraphicsContext::onImageAcquired(const uint32_t imageId)
{
    _current_image = imageId;
    const VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();

    // A different in-flight frame may still be using this image (image acquisition order is not guaranteed to be
    // round-robin). Wait it out before reusing the image's command buffer, framebuffer and render-complete semaphore.
    if(_images_in_flight[imageId] != VK_NULL_HANDLE)
        VKUtil::checkResult(vkWaitForFences(vkLogicalDevice, 1, &_images_in_flight[imageId], VK_TRUE, UINT64_MAX));
    _images_in_flight[imageId] = _in_flight_fences[_current_frame];

    // Reset only after every wait that might target this fence, immediately before it is handed to vkQueueSubmit.
    VKUtil::checkResult(vkResetFences(vkLogicalDevice, 1, &_in_flight_fences[_current_frame]));
}

void VKGraphicsContext::submit(const VkQueue queue)
{
    _submit_queue.submit(queue, _render_complete[_current_image], _in_flight_fences[_current_frame]);
}

VkSemaphore VKGraphicsContext::currentRenderComplete() const
{
    return _render_complete[_current_image];
}

void VKGraphicsContext::endFrame()
{
    _current_frame = (_current_frame + 1) % kMaxFramesInFlight;
}

VKGraphicsContext::State::State(sp<RenderPassPhrase> renderPassPhrase, const VkCommandBuffer commandBuffer, const bool commandBufferBegan)
    : _render_pass_phrase(std::move(renderPassPhrase)), _command_buffer(commandBuffer), _command_buffer_began(commandBufferBegan), _render_pass(VK_NULL_HANDLE)
{
}

VkRenderPass VKGraphicsContext::State::ensureRenderPass()
{
    const VkRenderPass renderPass = _render_pass_phrase->acquire();
    ensureRenderPassCommandBuffer();
    return renderPass;
}

VkCommandBuffer VKGraphicsContext::State::ensureRenderPassCommandBuffer()
{
    if(_render_pass == VK_NULL_HANDLE)
    {
        if(!_command_buffer_began)
        {
            constexpr VkCommandBufferBeginInfo cmdBufInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            VKUtil::checkResult(vkBeginCommandBuffer(_command_buffer, &cmdBufInfo));
            _command_buffer_began = true;
        }
        _render_pass = _render_pass_phrase->begin(_command_buffer);
    }
    return _command_buffer;
}

}
