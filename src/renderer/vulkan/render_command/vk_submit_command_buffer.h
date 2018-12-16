#ifndef ARK_RENDERER_VULKAN_RENDER_COMMAND_VK_COMMAND_BUFFER_DRAW_INDEXED_H_
#define ARK_RENDERER_VULKAN_RENDER_COMMAND_VK_COMMAND_BUFFER_DRAW_INDEXED_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "graphics/inf/render_command.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKSubmitCommandBuffer final : public RenderCommand {
public:
    VKSubmitCommandBuffer(sp<VKRenderTarget> renderTarget, VkCommandBuffer commandBuffer);

    virtual void draw(GraphicsContext& graphicsContext) override;

private:
    sp<VKRenderTarget> _render_target;
    VkCommandBuffer _command_buffer;
};

}
}

#endif
