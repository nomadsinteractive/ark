#ifndef ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_

#include "core/base/notifier.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/buffer.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_memory_ptr.h"
#include "renderer/vulkan/util/vulkan_buffer.hpp"

namespace ark {
namespace vulkan {

class VKBuffer : public Buffer::Delegate {
public:
    VKBuffer(sp<VKRenderer> renderer, sp<Recycler> recycler, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
    ~VKBuffer() override;
    DISALLOW_COPY_AND_ASSIGN(VKBuffer);

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual void uploadBuffer(GraphicsContext& graphicsContext, Input& input) override;
    virtual ResourceRecycleFunc recycle() override;

    void reload(GraphicsContext& graphicsContext, const ByteArray::Borrowed& buf);

    const VkBuffer& vkBuffer() const;

    VkResult invalidate();

    const VkDescriptorBufferInfo& vkDescriptor() const;

    Notifier& notifier();

private:
    void* map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void allocateMemory(GraphicsContext& graphicsContext, const VkMemoryRequirements& memReqs);
    void ensureSize(GraphicsContext& graphicsContext, size_t size);

    void bind();
    VkResult flush();

    bool isDeviceLocal() const;
    bool isHostCoherent() const;

private:
    sp<VKRenderer> _renderer;
    sp<Recycler> _recycler;

    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags _usage_flags;
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags _memory_property_flags;

    VkDescriptorBufferInfo _descriptor;

    VKMemoryPtr _memory;
    VkMemoryRequirements _memory_requirements;

    Notifier _notifier;
};

}
}

#endif
