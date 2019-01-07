#include "renderer/vulkan/base/vk_descriptor_pool.h"

#include "renderer/base/recycler.h"

#include "renderer/vulkan/base/vk_device.h"

namespace ark {
namespace vulkan {

VKDescriptorPool::VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, VkDescriptorPool descriptorPool)
    : _recycler(recycler), _device(device), _descriptor_pool(descriptorPool)
{
}

VKDescriptorPool::~VKDescriptorPool()
{
    VkDescriptorPool descriptorPool = _descriptor_pool;
    const sp<VKDevice> device = std::move(_device);
    _descriptor_pool = VK_NULL_HANDLE;
    _recycler->recycle([device, descriptorPool](GraphicsContext&) {
        vkDestroyDescriptorPool(device->vkLogicalDevice(), descriptorPool, nullptr);
    });
}

VkDescriptorPool VKDescriptorPool::vkDescriptorPool() const
{
    return _descriptor_pool;
}

}
}
