#ifndef ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_
#define ARK_RENDERER_VULKAN_BASE_VK_BUFFER_H_

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/vulkan/base/device.h"
#include "renderer/vulkan/util/vulkan_buffer.hpp"

namespace ark {
namespace vulkan {

class VKBuffer {
public:
    VKBuffer(const sp<Device>& device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size);
    ~VKBuffer();

    DISALLOW_COPY_AND_ASSIGN(VKBuffer);

    void* map(VkDeviceSize _size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
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
    * Copies the specified data to the mapped buffer
    *
    * @param data Pointer to the data to copy
    * @param size Size of the data to copy in machine units
    *
    */
    void upload(void* data, VkDeviceSize size);

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
    /**
    * Release all Vulkan resources held by this buffer
    */
    void destroy();

private:
    sp<Device> _device;

    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags usageFlags;
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags memoryPropertyFlags;

    VkBuffer _buffer;
    VkDeviceMemory _memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo _descriptor;
    VkDeviceSize _size = 0;
    VkDeviceSize _alignment = 0;
};

}
}

#endif
