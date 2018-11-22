#include "renderer/vulkan/base/buffer.h"

#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

Buffer::Buffer(const sp<Device>& device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size)
    : _device(device), usageFlags(usageFlags), memoryPropertyFlags(memoryPropertyFlags)
{
    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
    VulkanAPI::checkResult(vkCreateBuffer(_device->logicalDevice(), &bufferCreateInfo, nullptr, &_buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(_device->logicalDevice(), _buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    VulkanAPI::checkResult(vkAllocateMemory(_device->logicalDevice(), &memAlloc, nullptr, &_memory));

    _alignment = memReqs.alignment;
    _size = memAlloc.allocationSize;

    setupDescriptor();

    bind();
}

Buffer::~Buffer()
{
    destroy();
}

void* Buffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    void* mapped = nullptr;
    VulkanAPI::checkResult(vkMapMemory(_device->logicalDevice(), _memory, offset, size, 0, &mapped));
    return mapped;
}

void Buffer::unmap(void* mapped)
{
    DASSERT(mapped);
    vkUnmapMemory(_device->logicalDevice(), _memory);
}

VkResult Buffer::bind(VkDeviceSize offset)
{
    return vkBindBufferMemory(_device->logicalDevice(), _buffer, _memory, offset);
}

void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    _descriptor.offset = offset;
    _descriptor.buffer = _buffer;
    _descriptor.range = size;
}

void Buffer::upload(void* data, VkDeviceSize size)
{
    void* mapped = map();
    memcpy(mapped, data, size);
    if((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        flush();
    unmap(mapped);
}

VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(_device->logicalDevice(), 1, &mappedRange);
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(_device->logicalDevice(), 1, &mappedRange);
}

const VkDescriptorBufferInfo&Buffer::descriptor() const
{
    return _descriptor;
}

void Buffer::destroy()
{
    if (_buffer)
        vkDestroyBuffer(_device->logicalDevice(), _buffer, nullptr);
    if (_memory)
        vkFreeMemory(_device->logicalDevice(), _memory, nullptr);
}

}
}
