#ifndef ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_

#include <vulkan/vulkan.h>

#include "core/epi/notifier.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/buffer.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/util/vulkan_buffer.hpp"

namespace ark {
namespace vulkan {

class VKBuffer : public Buffer::Delegate {
public:
    VKBuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, const sp<Uploader>& uploader, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
    ~VKBuffer() override;
    DISALLOW_COPY_AND_ASSIGN(VKBuffer);

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

    virtual void reload(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader) override;

    const VkBuffer& vkBuffer() const;

    /**
    * Invalidate a memory range of the buffer to make it visible to the host
    *
    * @note Only required for non-coherent memory
    *
    * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
    * @param offset (Optional) Byte offset from beginning
    *
    * @return VkResult of the invalidate call
    */
    VkResult invalidate(VkDeviceSize _size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    const VkDescriptorBufferInfo& descriptor() const;

    Notifier& notifier();

private:
    void* map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap(void* mapped);

    void allocateMemory(const VkMemoryRequirements& memReqs);
    void ensureSize(const Uploader& uploader);

    void bind(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /**
    * Flush a memory range of the buffer to make it visible to the device
    *
    * @note Only required for non-coherent memory
    *
    * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
    * @param offset (Optional) Byte offset from beginning
    *
    * @return VkResult of the flush call
    */
    VkResult flush(VkDeviceSize _size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

private:
    sp<VKRenderer> _renderer;
    sp<Recycler> _recycler;
    sp<Uploader> _uploader;

    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags _usage_flags;
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags _memory_property_flags;

    VkDeviceMemory _memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo _descriptor;

    VkMemoryAllocateInfo _memory_allocation_info;

    Notifier _notifier;
};

}
}

#endif
