#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKDescriptorPool {
public:
    VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, VkDescriptorPool descriptorPool);
    VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, std::map<VkDescriptorType, uint32_t> poolSizes);
    ~VKDescriptorPool();

    void upload(GraphicsContext& graphicsContext);

    VkDescriptorPool vkDescriptorPool() const;

private:
    sp<Recycler> _recycler;
    sp<VKDevice> _device;

    std::map<VkDescriptorType, uint32_t> _pool_sizes;

    VkDescriptorPool _descriptor_pool;
};

}
