#ifndef ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_H_
#define ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKTexture {
public:
    VKTexture(const sp<ResourceManager>& resourceManager, const sp<CommandPool>& commandPool);
    ~VKTexture();

    const VkDescriptorImageInfo& descriptor() const;

private:
    void loadTexture();

private:
    sp<ResourceManager> _resource_manager;
    sp<CommandPool> _command_pool;
    sp<Device> _device;

    VkImage _image;
    VkDeviceMemory _memory;
    uint32_t _width, _height;
    uint32_t _mip_levels;
    VkDescriptorImageInfo _descriptor;
};


}
}

#endif
