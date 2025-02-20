#include "vulkan/base/vk_command_buffers.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/recycler.h"

#include "vulkan/base/vk_command_pool.h"
#include "vulkan/base/vk_swap_chain.h"

namespace ark::plugin::vulkan {

VKCommandBuffers::VKCommandBuffers(const sp<Recycler>& recycler, const VKSwapChain& renderTarget)
    : _recycler(recycler), _command_pool(renderTarget.commandPool()), _command_buffers(renderTarget.makeCommandBuffers())
{
}

VKCommandBuffers::~VKCommandBuffers()
{
    sp<VKCommandPool> commandPool = std::move(_command_pool);
    std::vector<VkCommandBuffer> commandBuffers = std::move(_command_buffers);
    _recycler->recycle([commandPool, commandBuffers](GraphicsContext&) {
        commandPool->destroyCommandBuffers(static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    });
}

const std::vector<VkCommandBuffer>& VKCommandBuffers::vkCommandBuffers() const
{
    return _command_buffers;
}

}
