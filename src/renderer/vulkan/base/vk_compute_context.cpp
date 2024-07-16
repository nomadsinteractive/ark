#include "renderer/vulkan/base/vk_compute_context.h"

#include "graphics/base/color.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark::vulkan {

VKComputeContext::VKComputeContext(GraphicsContext& graphicsContext, sp<VKRenderer> renderer)
    : _renderer(std::move(renderer)), _command_pool(_renderer->device()->makeComputeCommandPool()), _submit_queue(_renderer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT), _command_buffer(VK_NULL_HANDLE),
      _semaphore_render_complete(VK_NULL_HANDLE), _semaphore_compute_complete(_submit_queue.createSignalSemaphore())
{
}

void VKComputeContext::begin()
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    DASSERT(_semaphore_render_complete);
    _command_buffer = _command_pool->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    _submit_queue.begin(_semaphore_render_complete);
}

void VKComputeContext::end()
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    VKUtil::checkResult(vkEndCommandBuffer(_command_buffer));
    _submit_queue.submitCommandBuffer(_command_buffer);
}

void VKComputeContext::submit()
{
    _submit_queue.submit(_command_pool->vkQueue());
    _command_pool->destroyCommandBuffers(1, &_command_buffer);
    _command_buffer = VK_NULL_HANDLE;
}

VkCommandBuffer VKComputeContext::start(GraphicsContext& graphicsContext)
{
    if(_semaphore_render_complete == VK_NULL_HANDLE)
        _semaphore_render_complete = graphicsContext.attachments().ensure<VKGraphicsContext>()->submitQueue().createSignalSemaphore();
    if(_command_buffer == VK_NULL_HANDLE)
        begin();
    return _command_buffer;
}

VkCommandBuffer VKComputeContext::vkCommandBuffer() const
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    return _command_buffer;
}

VkSemaphore VKComputeContext::semaphoreComputeComplete() const
{
    return _semaphore_compute_complete;
}

}
