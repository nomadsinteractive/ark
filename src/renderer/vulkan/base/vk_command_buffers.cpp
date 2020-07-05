#include "renderer/vulkan/base/vk_command_buffers.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/recycler.h"

#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKCommandBuffers::VKCommandBuffers(const sp<Recycler>& recycler, const sp<VKRenderTarget>& renderTarget)
    : _recycler(recycler), _render_target(renderTarget), _command_buffers(renderTarget->makeCommandBuffers())
{
}

VKCommandBuffers::~VKCommandBuffers()
{
    sp<VKRenderTarget> renderTarget = std::move(_render_target);
    std::vector<VkCommandBuffer> commandBuffers = std::move(_command_buffers);
    _recycler->recycle([renderTarget, commandBuffers](GraphicsContext&) {
        renderTarget->commandPool()->destroyCommandBuffers(static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    });
}

const std::vector<VkCommandBuffer>& VKCommandBuffers::vkCommandBuffers() const
{
    return _command_buffers;
}

}
}
