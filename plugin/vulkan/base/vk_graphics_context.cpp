#include "vulkan/base/vk_graphics_context.h"

#include "graphics/base/color.h"
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
    MainRenderPassPhrase(const RenderEngineContext::Resolution& resolution, const sp<VKRenderer>& renderer, VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, const Color& backgroundColor)
        : RenderPassPhrase(resolution, 1, commandBuffer), _renderer(renderer), _framebuffer(framebuffer), _clear_color_value(toVkClearColorValue(backgroundColor.rgba())) {
    }

    VkRenderPass acquire() override {
        const VKSwapChain& renderTarget = _renderer->renderTarget();
        return renderTarget.vkRenderPassBeginInfo().renderPass;
    }

    VkRenderPass begin(VkCommandBuffer commandBuffer) override {
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

VKGraphicsContext::VKGraphicsContext(GraphicsContext& graphicsContext, const sp<VKRenderer>& renderer)
    : _renderer(renderer), _render_target(_renderer->renderTarget()), _command_buffers(sp<VKCommandBuffers>::make(graphicsContext.recycler(), _render_target)),
      _submit_queue(_renderer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT), _semaphore_render_complete(_submit_queue.createSignalSemaphore())
{
    VkDevice vkLogicalDevice = _renderer->vkLogicalDevice();
    // Create synchronization objects
    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    VKUtil::checkResult(vkCreateSemaphore(vkLogicalDevice, &semaphoreCreateInfo, nullptr, &_semaphore_present_complete));
}

VKGraphicsContext::~VKGraphicsContext()
{
    vkDestroySemaphore(_renderer->vkLogicalDevice(), _semaphore_present_complete, nullptr);
}

void VKGraphicsContext::begin(const uint32_t imageId, const Color& backgroundColor)
{
    const VKSwapChain& renderTarget = _renderer->renderTarget();
    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();

    VkCommandBuffer commandBuffer = commandBuffers.at(imageId);
    _state_stack.push(State(sp<RenderPassPhrase>::make<MainRenderPassPhrase>(RenderEngineContext::Resolution{_render_target->width(), _render_target->height()}, _renderer, commandBuffer, renderTarget.frameBuffers().at(imageId), backgroundColor), commandBuffer, false));

    constexpr VkCommandBufferBeginInfo cmdBufInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VKUtil::checkResult(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

    _submit_queue.begin(_semaphore_present_complete);
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

VKGraphicsContext::RenderPassPhrase::RenderPassPhrase(const RenderEngineContext::Resolution& resolution, const uint32_t colorAttachmentCount, const VkCommandBuffer commandBuffer)
    : _resolution(resolution), _color_attachment_count(colorAttachmentCount), _command_buffer(commandBuffer)
{
}

const RenderEngineContext::Resolution& VKGraphicsContext::RenderPassPhrase::resolution() const
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

void VKGraphicsContext::pushState(sp<RenderPassPhrase> starter)
{
    DCHECK(!_state_stack.empty(), "First state must be push by VKGraphicsContext::begin");
    const bool beginCommandBuffer = _state_stack.top()._render_pass != VK_NULL_HANDLE;
    const VkCommandBuffer commandBuffer = beginCommandBuffer ? starter->vkCommandBuffer() : _state_stack.top()._command_buffer;
    _state_stack.push(State(std::move(starter), commandBuffer, beginCommandBuffer));
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

void VKGraphicsContext::submit(VkQueue queue)
{
    _submit_queue.submit(queue);
}

const sp<VKSemaphore>& VKGraphicsContext::semaphoreRenderComplete() const
{
    return _semaphore_render_complete;
}

VkSemaphore VKGraphicsContext::semaphorePresentComplete() const
{
    return _semaphore_present_complete;
}

VKGraphicsContext::State::State(sp<RenderPassPhrase> renderPassPhrase, const VkCommandBuffer commandBuffer, const bool beginCommandBuffer)
    : _render_pass_phrase(std::move(renderPassPhrase)), _command_buffer(commandBuffer), _begin_command_buffer(beginCommandBuffer), _render_pass(VK_NULL_HANDLE)
{
}

const sp<VKGraphicsContext::RenderPassPhrase>& VKGraphicsContext::State::renderPassPhrase() const
{
    return _render_pass_phrase;
}

VkCommandBuffer VKGraphicsContext::State::commandBuffer() const
{
    return _command_buffer;
}

VkRenderPass VKGraphicsContext::State::acquireRenderPass()
{
    const VkRenderPass renderPass = _render_pass_phrase->acquire();
    ensureRenderPass();
    return renderPass;
}

VkCommandBuffer VKGraphicsContext::State::ensureRenderPass()
{
    if(_render_pass == VK_NULL_HANDLE)
    {
        if(_begin_command_buffer)
        {
            constexpr VkCommandBufferBeginInfo cmdBufInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            VKUtil::checkResult(vkBeginCommandBuffer(_command_buffer, &cmdBufInfo));
        }
        _render_pass = _render_pass_phrase->begin(_command_buffer);
    }
    return _command_buffer;
}

}
