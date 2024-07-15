#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKRenderer {
public:
    VKRenderer() = default;
    ~VKRenderer();

    const sp<VKCommandPool>& commandPool() const;
    const sp<VKDevice>& device() const;
    const sp<VKHeap>& heap() const;
    const sp<VKRenderTarget>& renderTarget() const;

    VkDevice vkLogicalDevice() const;
    VkPhysicalDevice vkPhysicalDevice() const;

private:
    sp<VKInstance> _instance;
    sp<VKDevice> _device;
    sp<VKHeap> _heap;
    sp<VKRenderTarget> _render_target;

    friend class RendererFactoryVulkan;
};


}
