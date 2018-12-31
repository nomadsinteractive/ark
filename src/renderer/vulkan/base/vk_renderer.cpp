#include "renderer/vulkan/base/vk_renderer.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"

namespace ark {
namespace vulkan {

const sp<VKCommandPool>& VKRenderer::commandPool() const
{
    DCHECK(_render_target, "VKRenderer uninitialized");
    return _render_target->commandPool();
}

const sp<VKDevice>& VKRenderer::device() const
{
    DCHECK(_device, "VKRenderer uninitialized");
    return _device;
}

const sp<VKRenderTarget>& VKRenderer::renderTarget() const
{
    DCHECK(_render_target, "VKRenderer uninitialized");
    return _render_target;
}

//VkDescriptorPool VKRenderer::vkDescriptorPool() const
//{
//    DCHECK(_render_target, "VKRenderer uninitialized");
//    return _render_target->vkDescriptorPool();
//}

VkDevice VKRenderer::vkLogicalDevice() const
{
    DCHECK(_device, "VKRenderer uninitialized");
    return _device->logicalDevice();
}

VkPhysicalDevice VKRenderer::vkPhysicalDevice() const
{
    DCHECK(_device, "VKRenderer uninitialized");
    return _device->physicalDevice();
}

VkRenderPass VKRenderer::vkRenderPass() const
{
    DCHECK(_render_target, "VKRenderer uninitialized");
    return _render_target->vkRenderPass();
}

}
}
