#include "renderer/vulkan/base/vk_graphics_context.h"

#include "graphics/base/color.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_swap_chain.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark::vulkan {

namespace {

VkClearColorValue toVkClearColorValue(const V4& rgba)
{
    return {{rgba.x(), rgba.y(), rgba.z(), rgba.w()}};
}

class MainRenderPassPhrase final : public VKGraphicsContext::RenderPassPhrase {
public:
    MainRenderPassPhrase(const sp<VKRenderer>& renderer, VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, const Color& backgroundColor)
        : _renderer(renderer), _command_buffer(commandBuffer), _framebuffer(framebuffer), _clear_color_value(toVkClearColorValue(backgroundColor.rgba())) {
    }

    virtual VkCommandBuffer vkCommandBuffer() override {
        return _command_buffer;
    }

    virtual VkRenderPass create(const PipelineDescriptor& /*bindings*/) override {
        const VKSwapChain& renderTarget = _renderer->renderTarget();
        return renderTarget.vkRenderPassBeginInfo().renderPass;
    }

    virtual VkRenderPass begin(VkCommandBuffer commandBuffer) override {
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
    VkCommandBuffer _command_buffer;
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

void VKGraphicsContext::begin(uint32_t imageId, const Color& backgroundColor)
{
    const VKSwapChain& renderTarget = _renderer->renderTarget();
    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->vkCommandBuffers();

    VkCommandBuffer commandBuffer = commandBuffers.at(imageId);
    _state_stack.push(State(sp<MainRenderPassPhrase>::make(_renderer, commandBuffer, renderTarget.frameBuffers().at(imageId), backgroundColor), commandBuffer, false));

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

VKGraphicsContext::State& VKGraphicsContext::getCurrentState()
{
    DASSERT(!_state_stack.empty());
    return _state_stack.top();
}

void VKGraphicsContext::pushState(sp<RenderPassPhrase> starter)
{
    DCHECK(!_state_stack.empty(), "First state must be push by VKGraphicsContext::begin");
    const bool beginCommandBuffer = _state_stack.top()._render_pass != VK_NULL_HANDLE;
    const VkCommandBuffer commandBuffer = beginCommandBuffer ? starter->vkCommandBuffer() : _state_stack.top()._command_buffer;
    _state_stack.push(State(std::move(starter), commandBuffer, beginCommandBuffer));
}

void VKGraphicsContext::popState()
{
    DASSERT(!_state_stack.empty());
    State state = _state_stack.top();
    _state_stack.pop();

    VkCommandBuffer commandBuffer = state._command_buffer;
    if(state._render_pass != VK_NULL_HANDLE)
        vkCmdEndRenderPass(commandBuffer);

    if(_state_stack.empty() || _state_stack.top()._command_buffer != commandBuffer)
    {
        VKUtil::checkResult(vkEndCommandBuffer(commandBuffer));
        _submit_queue.submitCommandBuffer(commandBuffer);
    }
}

void VKGraphicsContext::submit(VkQueue queue)
{
    _submit_queue.submit(queue);
}

void VKGraphicsContext::addSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    _submit_queue.addSubmitInfo(commandBufferCount, pCommandBuffers);
}

void VKGraphicsContext::addWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitStageFlag)
{
    _submit_queue.addWaitSemaphore(semaphore, waitStageFlag);
}

VkSemaphore VKGraphicsContext::semaphoreRenderComplete() const
{
    return _semaphore_render_complete;
}

VkSemaphore VKGraphicsContext::semaphorePresentComplete() const
{
    return _semaphore_present_complete;
}

VKGraphicsContext::State::State(sp<VKGraphicsContext::RenderPassPhrase> renderPassPhrase, VkCommandBuffer commandBuffer, bool beginCommandBuffer)
    : _render_pass_phrase(std::move(renderPassPhrase)), _command_buffer(commandBuffer), _begin_command_buffer(beginCommandBuffer), _render_pass(VK_NULL_HANDLE)
{
}

VkRenderPass VKGraphicsContext::State::createRenderPass(const PipelineDescriptor& bindings)
{
    return _render_pass_phrase->create(bindings);
}

VkCommandBuffer VKGraphicsContext::State::startRecording()
{
    if(_render_pass == VK_NULL_HANDLE)
    {
        if(_begin_command_buffer)
        {
            constexpr VkCommandBufferBeginInfo cmdBufInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            VKUtil::checkResult(vkBeginCommandBuffer(_command_buffer, &cmdBufInfo));
        }
        _render_pass = _render_pass_phrase->begin(_command_buffer);
    }
    return _command_buffer;
}

}
