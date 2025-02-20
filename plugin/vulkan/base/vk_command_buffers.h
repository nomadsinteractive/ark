#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKCommandBuffers {
public:
    VKCommandBuffers(const sp<Recycler>& recycler, const VKSwapChain& renderTarget);
    ~VKCommandBuffers();

    const std::vector<VkCommandBuffer>& vkCommandBuffers() const;

private:
    sp<Recycler> _recycler;
    sp<VKCommandPool> _command_pool;
    std::vector<VkCommandBuffer> _command_buffers;
};

}
