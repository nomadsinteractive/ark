#ifndef ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_2D_H_
#define ARK_RENDERER_VULKAN_BASE_VK_TEXTURE_2D_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKTexture2D : public Resource {
public:
    VKTexture2D(const sp<Recycler>& recycler, const sp<VKCommandPool>& commandPool, const sp<Variable<bitmap>>& bitmap);
    ~VKTexture2D() override;

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

    const VkDescriptorImageInfo& descriptor() const;

private:
    void doUpload();

private:
    sp<Recycler> _recycler;
    sp<VKCommandPool> _command_pool;
    sp<Variable<bitmap>> _bitmap;
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
