#include "renderer/vulkan/base/vk_buffer.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

VKBuffer::VKBuffer(const sp<VKDevice>& device, const sp<Recycler>& recycler, const sp<Uploader>& uploader, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
    : Buffer::Delegate(uploader->size()), _device(device), _recycler(recycler), _uploader(uploader), _usage_flags(usageFlags), _memory_property_flags(memoryPropertyFlags)
{
    const VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, _size);
    VulkanAPI::checkResult(vkCreateBuffer(_device->logicalDevice(), &bufferCreateInfo, nullptr, &_buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(_device->logicalDevice(), _buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    VulkanAPI::checkResult(vkAllocateMemory(_device->logicalDevice(), &memAlloc, nullptr, &_memory));

    setupDescriptor();
    bind();
}

VKBuffer::~VKBuffer()
{
    _recycler->recycle(*this);
}

uint32_t VKBuffer::id()
{
    return 0;
}

void VKBuffer::upload(GraphicsContext& graphicsContext)
{
    reload(graphicsContext, _uploader);
}

Resource::RecycleFunc VKBuffer::recycle()
{
    const sp<VKDevice> device = _device;
    VkBuffer buffer = _buffer;
    VkDeviceMemory memory = _memory;

    _buffer = VK_NULL_HANDLE;
    _memory = VK_NULL_HANDLE;

    return [device, buffer, memory](GraphicsContext&) {
        if (buffer)
            vkDestroyBuffer(device->logicalDevice(), buffer, nullptr);
        if (memory)
            vkFreeMemory(device->logicalDevice(), memory, nullptr);
    };
}

void VKBuffer::reload(GraphicsContext& /*graphicsContext*/, const sp<Uploader>& transientUploader)
{
    void* mapped = map();
    size_t offset = 0;
    transientUploader->upload([mapped, &offset](void* buf, size_t size) {
        memcpy(reinterpret_cast<int8_t*>(mapped) + offset, buf, size);
        offset += size;
    });
    if((_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        flush();
    unmap(mapped);
}

const VkBuffer& VKBuffer::vkBuffer() const
{
    return _buffer;
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

const VkDescriptorBufferInfo& VKBuffer::descriptor() const
{
    return _descriptor;
}

void VKBuffer::destroy()
{
    const sp<VKDevice> device = _device;
    VkBuffer buffer = _buffer;
    VkDeviceMemory memory = _memory;

    if (buffer)
        vkDestroyBuffer(device->logicalDevice(), buffer, nullptr);
    if (memory)
        vkFreeMemory(device->logicalDevice(), memory, nullptr);
}

}
}
