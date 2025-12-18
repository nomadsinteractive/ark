#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKRenderer {
public:
    VKRenderer() = default;
    ~VKRenderer();

    const sp<VKCommandPool>& commandPool() const;
    const sp<VKInstance>& instance() const;
    const sp<VKDevice>& device() const;
    const sp<VKHeap>& heap() const;
    const sp<VKSwapChain>& renderTarget() const;

    VkDevice vkLogicalDevice() const;
    VkPhysicalDevice vkPhysicalDevice() const;

private:
    sp<VKInstance> _instance;
    sp<VKDevice> _device;
    sp<VKHeap> _heap;
    sp<VKSwapChain> _render_target;

    friend class RendererFactoryVulkan;
};

}
