#ifndef ARK_RENDERER_VULKAN_BASE_VK_MEMORY_H_
#define ARK_RENDERER_VULKAN_BASE_VK_MEMORY_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKMemory : public Resource {
public:
    VKMemory(const sp<VKRenderer>& renderer, VkDeviceSize size, uint32_t memoryType);
    ~VKMemory() override;

    VkDeviceSize size() const;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    void* map(VkDeviceSize offset, VkDeviceSize size);
    void unmap();

    VkDeviceMemory vkMemory() const;

private:
    sp<VKRenderer> _renderer;

    VkDeviceMemory _memory;
    VkMemoryAllocateInfo _allocation_info;

};


}
}

#endif
