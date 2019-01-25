#ifndef ARK_RENDERER_VULKAN_BASE_VK_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_VULKAN_BASE_VK_GRAPHICS_CONTEXT_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKGraphicsContext {
public:
    VKGraphicsContext(const sp<VKRenderer>& renderer);

    void initialize(GraphicsContext& graphicsContext);

    void begin(uint32_t imageId, const Color& backgroundColor);
    void end();
    void submit(GraphicsContext& graphicsContext);

    VkCommandBuffer vkCommandBuffer() const;

private:
    sp<VKRenderer> _renderer;

    sp<VKCommandBuffers> _command_buffers;

    VkCommandBuffer _command_buffer;
};

}
}

#endif
