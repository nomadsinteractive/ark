#include "vulkan/base/vk_descriptor_pool.h"

#include "renderer/base/recycler.h"

#include "vulkan/base/vk_device.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

VKDescriptorPool::VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, Map<VkDescriptorType, uint32_t> poolSizes, const uint32_t maxSets)
    : _recycler(recycler), _device(device), _pool_sizes(std::move(poolSizes)), max_sets(maxSets), _descriptor_pool(VK_NULL_HANDLE)
{
}

VKDescriptorPool::~VKDescriptorPool()
{
    VkDescriptorPool descriptorPool = _descriptor_pool;
    _descriptor_pool = VK_NULL_HANDLE;
    _recycler->recycle([device = std::move(_device), descriptorPool](GraphicsContext&) {
        vkDestroyDescriptorPool(device->vkLogicalDevice(), descriptorPool, nullptr);
    });
}

void VKDescriptorPool::upload(GraphicsContext& /*graphicsContext*/)
{
    Vector<VkDescriptorPoolSize> poolSizes;

    for(const auto& [k, v] : _pool_sizes)
        poolSizes.push_back(vks::initializers::descriptorPoolSize(k, v));

    const VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, max_sets);
    VKUtil::checkResult(vkCreateDescriptorPool(_device->vkLogicalDevice(), &descriptorPoolInfo, nullptr, &_descriptor_pool));
}

VkDescriptorPool VKDescriptorPool::vkDescriptorPool() const
{
    return _descriptor_pool;
}

}
