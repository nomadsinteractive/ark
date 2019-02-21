#ifndef ARK_RENDERER_VULKAN_BASE_VK_COMMAND_BUFFERS_H_
#define ARK_RENDERER_VULKAN_BASE_VK_COMMAND_BUFFERS_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKCommandBuffers {
public:
    VKCommandBuffers(const sp<Recycler>& recycler, const sp<VKRenderTarget>& renderTarget);
    ~VKCommandBuffers();

    const std::vector<VkCommandBuffer>& vkCommandBuffers() const;

    void submit(GraphicsContext& graphicsContext) const;

private:
    sp<Recycler> _recycler;
    sp<VKRenderTarget> _render_target;
    std::vector<VkCommandBuffer> _command_buffers;
};

}
}

#endif
