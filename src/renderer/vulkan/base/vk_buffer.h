#ifndef ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/base/buffer.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/util/vulkan_buffer.hpp"

namespace ark {
namespace vulkan {

class VKBuffer : public Buffer::Delegate {
public:
    VKBuffer(const sp<VKDevice>& device, const sp<Recycler>& recycler, const sp<Uploader>& uploader, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
    ~VKBuffer() override;
    DISALLOW_COPY_AND_ASSIGN(VKBuffer);

    virtual uint32_t id() override;
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

private:
    void* map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap(void* mapped);

    /**
    * Attach the allocated memory block to the buffer
    *
    * @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
    *
    * @return VkResult of the bindBufferMemory call
    */
    VkResult bind(VkDeviceSize offset = 0);

    /**
    * Setup the default descriptor for this buffer
    *
    * @param size (Optional) Size of the memory range of the descriptor
    * @param offset (Optional) Byte offset from beginning
    *
    */
    void setupDescriptor(VkDeviceSize _size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

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

    /**
    * Release all Vulkan resources held by this buffer
    */
    void destroy();

private:
    sp<VKDevice> _device;
    sp<Recycler> _recycler;
    sp<Uploader> _uploader;

    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags _usage_flags;
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags _memory_property_flags;

    VkBuffer _buffer;
    VkDeviceMemory _memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo _descriptor;
};

}
}

#endif
