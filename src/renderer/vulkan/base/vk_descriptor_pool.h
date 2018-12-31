#ifndef ARK_RENDERER_VULKAN_BASE_VK_DESCRIPTOR_POOL_H_
#define ARK_RENDERER_VULKAN_BASE_VK_DESCRIPTOR_POOL_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKDescriptorPool {
public:
    VKDescriptorPool(const sp<Recycler>& recycler, const sp<VKDevice>& device, VkDescriptorPool descriptorPool);
    ~VKDescriptorPool();

    VkDescriptorPool vkDescriptorPool() const;

private:
    sp<Recycler> _recycler;
    sp<VKDevice> _device;

    VkDescriptorPool _descriptor_pool;
};

}
}

#endif
