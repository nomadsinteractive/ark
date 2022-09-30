#ifndef ARK_RENDERER_VULKAN_BASE_VK_MEMORY_H_
#define ARK_RENDERER_VULKAN_BASE_VK_MEMORY_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/resource.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKMemory {
private:
    struct Stub  : public Resource {
        Stub(const sp<VKDevice>& device, const sp<Recycler>& recycler, VkDeviceSize size, uint32_t memoryType);
        ~Stub() override;

        virtual uint64_t id() override;
        virtual void upload(GraphicsContext& graphicsContext) override;
        virtual ResourceRecycleFunc recycle() override;

        sp<VKDevice> _device;
        sp<Recycler> _recycler;

        VkDeviceMemory _memory;
        VkMemoryAllocateInfo _allocation_info;
    };

public:
    VKMemory(const sp<VKDevice>& device, const sp<Recycler>& recycler, VkDeviceSize size, uint32_t memoryType);
    VKMemory(const sp<Stub>& stub);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(VKMemory);

    VkDeviceSize size() const;

    void upload(GraphicsContext& graphicsContext);

    void* map(VkDeviceSize offset, VkDeviceSize size);
    void unmap();

    VkDeviceMemory vkMemory() const;

    VKMemoryPtr begin() const;
    VKMemoryPtr end() const;

private:
    sp<Stub> _stub;

};


}
}

#endif
