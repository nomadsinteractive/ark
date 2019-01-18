#ifndef ARK_RENDERER_VULKAN_BASE_VK_RENDERER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_RENDERER_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKRenderer {
public:
    VKRenderer(const sp<ResourceManager>& resourceManager);
    ~VKRenderer();

    const sp<ResourceManager>& resourceManager() const;

    const sp<VKCommandPool>& commandPool() const;
    const sp<VKDevice>& device() const;
    const sp<VKRenderTarget>& renderTarget() const;

    VkDevice vkLogicalDevice() const;
    VkPhysicalDevice vkPhysicalDevice() const;
    VkRenderPass vkRenderPass() const;

private:
    sp<ResourceManager> _resource_manager;

    sp<VKInstance> _instance;
    sp<VKDevice> _device;
    sp<VKRenderTarget> _render_target;

    friend class RendererFactoryVulkan;
};


}
}

#endif
