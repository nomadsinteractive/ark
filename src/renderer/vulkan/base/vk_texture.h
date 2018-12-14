#ifndef ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_H_
#define ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKTexture : public Resource {
public:
    VKTexture(const sp<Recycler>& recycler, const sp<ResourceManager>& resourceManager, const sp<VKCommandPool>& commandPool);
    ~VKTexture();

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

    const VkDescriptorImageInfo& descriptor() const;

private:
    void loadTexture();

private:
    sp<Recycler> _recycler;
    sp<ResourceManager> _resource_manager;
    sp<VKCommandPool> _command_pool;
    sp<VKDevice> _device;

    VkImage _image;
    VkDeviceMemory _memory;
    uint32_t _width, _height;
    uint32_t _mip_levels;
    VkDescriptorImageInfo _descriptor;
};


}
}

#endif
