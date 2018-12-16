#include "renderer/vulkan/render_command/vk_submit_command_buffer.h"

#include "renderer/vulkan/base/vk_render_target.h"

namespace ark {
namespace vulkan {

VKSubmitCommandBuffer::VKSubmitCommandBuffer(sp<VKRenderTarget> renderTarget, VkCommandBuffer commandBuffer)
    : _render_target(std::move(renderTarget)), _command_buffer(commandBuffer)
{
}

void VKSubmitCommandBuffer::draw(GraphicsContext& /*graphicsContext*/)
{
    uint32_t currentBuffer = _render_target->acquire();
    _render_target->submit(&_command_buffer);
    _render_target->swap(currentBuffer);
}

}
}
