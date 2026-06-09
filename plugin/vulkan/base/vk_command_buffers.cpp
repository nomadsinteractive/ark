#include "vulkan/base/vk_command_buffers.h"

#include "core/types/owned_ptr.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/recyclable.h"

#include "vulkan/base/vk_command_pool.h"
#include "vulkan/base/vk_swap_chain.h"

namespace ark::plugin::vulkan {

namespace {

class RecyclableVKCommandBuffers final : public Recyclable {
public:
    RecyclableVKCommandBuffers(sp<VKCommandPool> commandPool, Vector<VkCommandBuffer> commandBuffers)
        : _command_pool(std::move(commandPool)), _command_buffers(std::move(commandBuffers)) {
    }

    ~RecyclableVKCommandBuffers() override {
        _command_pool->destroyCommandBuffers(static_cast<uint32_t>(_command_buffers.size()), _command_buffers.data());
    }

private:
    sp<VKCommandPool> _command_pool;
    Vector<VkCommandBuffer> _command_buffers;
};

}

VKCommandBuffers::VKCommandBuffers(const sp<Recycler>& recycler, const VKSwapChain& renderTarget)
    : _recycler(recycler), _command_pool(renderTarget.commandPool()), _command_buffers(renderTarget.makeCommandBuffers())
{
}

VKCommandBuffers::~VKCommandBuffers()
{
    _recycler->recycle(new RecyclableVKCommandBuffers(std::move(_command_pool), std::move(_command_buffers)));
}

const Vector<VkCommandBuffer>& VKCommandBuffers::vkCommandBuffers() const
{
    return _command_buffers;
}

}
