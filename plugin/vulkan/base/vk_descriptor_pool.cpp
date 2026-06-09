#include "vulkan/base/vk_descriptor_pool.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/recyclable.h"

#include "vulkan/base/vk_device.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

namespace {

class RecyclableVKDescriptorPool final : public Recyclable {
public:
    RecyclableVKDescriptorPool(sp<VKDevice> device, const VkDescriptorPool descriptorPool)
        : _device(std::move(device)), _descriptor_pool(descriptorPool) {
    }

    ~RecyclableVKDescriptorPool() override {
        vkDestroyDescriptorPool(_device->vkLogicalDevice(), _descriptor_pool, nullptr);
    }

private:
    sp<VKDevice> _device;
    VkDescriptorPool _descriptor_pool;
};

}

VKDescriptorPool::VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, Map<VkDescriptorType, uint32_t> poolSizes, const uint32_t maxSets)
    : _recycler(recycler), _device(device), _pool_sizes(std::move(poolSizes)), max_sets(maxSets), _descriptor_pool(VK_NULL_HANDLE)
{
}

VKDescriptorPool::~VKDescriptorPool()
{
    _recycler->recycle(new RecyclableVKDescriptorPool(std::move(_device), _descriptor_pool));
}

void VKDescriptorPool::upload(GraphicsContext& /*graphicsContext*/)
{
    Vector<VkDescriptorPoolSize> poolSizes;

    for(const auto& [k, v] : _pool_sizes)
        poolSizes.push_back(vks::initializers::descriptorPoolSize(k, v));

    const VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, max_sets, static_cast<uint32_t>(poolSizes.size()), poolSizes.data()};
    VKUtil::checkResult(vkCreateDescriptorPool(_device->vkLogicalDevice(), &descriptorPoolInfo, nullptr, &_descriptor_pool));
}

VkDescriptorPool VKDescriptorPool::vkDescriptorPool() const
{
    return _descriptor_pool;
}

}
