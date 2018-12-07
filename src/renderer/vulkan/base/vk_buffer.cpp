#include "renderer/vulkan/base/vk_buffer.h"

#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

VKBuffer::VKBuffer(const sp<Device>& device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size)
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

VKBuffer::~VKBuffer()
{
    destroy();
}

void* VKBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    void* mapped = nullptr;
    VulkanAPI::checkResult(vkMapMemory(_device->logicalDevice(), _memory, offset, size, 0, &mapped));
    return mapped;
}

void VKBuffer::unmap(void* mapped)
{
    DASSERT(mapped);
    vkUnmapMemory(_device->logicalDevice(), _memory);
}

VkResult VKBuffer::bind(VkDeviceSize offset)
{
    return vkBindBufferMemory(_device->logicalDevice(), _buffer, _memory, offset);
}

void VKBuffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    _descriptor.offset = offset;
    _descriptor.buffer = _buffer;
    _descriptor.range = size;
}

void VKBuffer::upload(void* data, VkDeviceSize size)
{
    void* mapped = map();
    memcpy(mapped, data, size);
    if((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        flush();
    unmap(mapped);
}

VkResult VKBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(_device->logicalDevice(), 1, &mappedRange);
}

VkResult VKBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(_device->logicalDevice(), 1, &mappedRange);
}

const VkDescriptorBufferInfo&VKBuffer::descriptor() const
{
    return _descriptor;
}

void VKBuffer::destroy()
{
    if (_buffer)
        vkDestroyBuffer(_device->logicalDevice(), _buffer, nullptr);
    if (_memory)
        vkFreeMemory(_device->logicalDevice(), _memory, nullptr);
}

}
}
