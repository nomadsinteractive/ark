#include "renderer/vulkan/base/vk_command_buffers.h"

#include "renderer/base/recycler.h"

#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

namespace {

class VKSubmitCommandBuffer final : public RenderCommand {
public:
    VKSubmitCommandBuffer(const sp<VKRenderTarget>& renderTarget, VkCommandBuffer commandBuffer)
        : _render_target(renderTarget), _command_buffer(commandBuffer) {
    }

    virtual void draw(GraphicsContext& /*graphicsContext*/) override {
        _render_target->submit(_command_buffer);
    }

private:
    sp<VKRenderTarget> _render_target;
    VkCommandBuffer _command_buffer;
};

}

VKCommandBuffers::VKCommandBuffers(const sp<Recycler>& recycler, const sp<VKRenderTarget>& renderTarget)
    : _recycler(recycler), _render_target(renderTarget), _command_buffers(renderTarget->makeCommandBuffers()), _render_commands(makeRenderCommands())
{
}

VKCommandBuffers::~VKCommandBuffers()
{
    sp<VKRenderTarget> renderTarget = std::move(_render_target);
    std::vector<VkCommandBuffer> commandBuffers = std::move(_command_buffers);
    _recycler->recycle([renderTarget, commandBuffers](GraphicsContext&) {
        renderTarget->commandPool()->destroyCommandBuffers(commandBuffers.size(), commandBuffers.data());
    });
}

const std::vector<VkCommandBuffer>& VKCommandBuffers::vkCommandBuffers() const
{
    return _command_buffers;
}

const sp<RenderCommand>& VKCommandBuffers::aquire() const
{
    uint32_t aquiredImageId = _render_target->aquiredImageId();
    return _render_commands.at(aquiredImageId);
}

void VKCommandBuffers::submit(GraphicsContext& /*graphicsContext*/) const
{
    uint32_t aquiredImageId = _render_target->aquiredImageId();
    _render_target->submit(_command_buffers.at(aquiredImageId));
}

std::vector<sp<RenderCommand>> VKCommandBuffers::makeRenderCommands() const
{
    std::vector<sp<RenderCommand>> renderCommands;
    for(VkCommandBuffer i : _command_buffers)
        renderCommands.push_back(sp<VKSubmitCommandBuffer>::make(_render_target, i));
    return renderCommands;
}

}
}
