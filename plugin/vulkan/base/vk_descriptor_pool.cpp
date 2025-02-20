#include "vulkan/base/vk_descriptor_pool.h"

#include "renderer/base/recycler.h"

#include "vulkan/base/vk_device.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

VKDescriptorPool::VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, VkDescriptorPool descriptorPool)
    : _recycler(recycler), _device(device), _descriptor_pool(descriptorPool)
{
}

VKDescriptorPool::VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, std::map<VkDescriptorType, uint32_t> poolSizes)
    : _recycler(recycler), _device(device), _pool_sizes(std::move(poolSizes)), _descriptor_pool(VK_NULL_HANDLE)
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

void VKDescriptorPool::upload(GraphicsContext& /*graphicsContext*/)
{
    std::vector<VkDescriptorPoolSize> poolSizes;

    for(const auto& i : _pool_sizes)
        poolSizes.push_back(vks::initializers::descriptorPoolSize(i.first, i.second));

    VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 1);
    VKUtil::checkResult(vkCreateDescriptorPool(_device->vkLogicalDevice(), &descriptorPoolInfo, nullptr, &_descriptor_pool));
}

VkDescriptorPool VKDescriptorPool::vkDescriptorPool() const
{
    return _descriptor_pool;
}

}
