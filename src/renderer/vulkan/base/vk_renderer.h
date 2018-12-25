#ifndef ARK_RENDERER_VULKAN_BASE_VK_RENDERER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_RENDERER_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKRenderer {
public:

    const sp<VKCommandPool>& commandPool() const;
    const sp<VKDevice>& device() const;
    const sp<VKRenderTarget>& renderTarget() const;

    VkDescriptorPool vkDescriptorPool() const;
    VkDevice vkLogicalDevice() const;
    VkPhysicalDevice vkPhysicalDevice() const;
    VkRenderPass vkRenderPass() const;


private:
    sp<VKInstance> _instance;
    sp<VKDevice> _device;
    sp<VKRenderTarget> _render_target;

    friend class RendererFactoryVulkan;
};


}
}

#endif
