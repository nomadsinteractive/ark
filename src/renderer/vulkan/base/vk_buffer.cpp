#include "renderer/vulkan/base/vk_buffer.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKBuffer::VKBuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
    : _renderer(renderer), _recycler(recycler), _usage_flags(usageFlags), _memory_property_flags(memoryPropertyFlags), _descriptor{},
      _memory_allocation_info(vks::initializers::memoryAllocateInfo())
{
}

VKBuffer::~VKBuffer()
{
    _recycler->recycle(*this);
}

uint64_t VKBuffer::id()
{
    return (uint64_t)(_descriptor.buffer);
}

void VKBuffer::upload(GraphicsContext& /*graphicsContext*/, const sp<Uploader>& uploader)
{
    if(uploader)
    {
        ensureSize(uploader);

        uint8_t* mapped = reinterpret_cast<uint8_t*>(map());
        uploader->upload([&mapped](void* buf, size_t size) {
            memcpy(mapped, buf, size);
            mapped += size;
        });
        if((_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            flush();
        unmap(mapped);
    }
}

Resource::RecycleFunc VKBuffer::recycle()
{
    const sp<VKDevice> device = _renderer->device();
    VkBuffer buffer = _descriptor.buffer;
    VkDeviceMemory memory = _memory;

    _memory_allocation_info.allocationSize = 0;
    _size = 0;
    _descriptor.buffer = VK_NULL_HANDLE;
    _memory = VK_NULL_HANDLE;

    return [device, buffer, memory](GraphicsContext&) {
        if (buffer)
            vkDestroyBuffer(device->vkLogicalDevice(), buffer, nullptr);
        if (memory)
            vkFreeMemory(device->vkLogicalDevice(), memory, nullptr);
    };
}

void VKBuffer::reload(GraphicsContext& /*graphicsContext*/, const bytearray& buf)
{
    DCHECK(buf->size() <= size(), "Buffer memory overflow, buffer size: %d, source size: %d", size(), buf->size());
    void* mapped = map();
    memcpy(mapped, buf->buf(), buf->size());
    if((_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        VKUtil::checkResult(flush());
    unmap(mapped);
}

const VkBuffer& VKBuffer::vkBuffer() const
{
    return _descriptor.buffer;
}

void* VKBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    void* mapped = nullptr;
    DCHECK(_memory, "Mapping to NULL memory");
    VKUtil::checkResult(vkMapMemory(_renderer->vkLogicalDevice(), _memory, offset, size, 0, &mapped));
    return mapped;
}

void VKBuffer::unmap(void* mapped)
{
    DASSERT(mapped);
    vkUnmapMemory(_renderer->vkLogicalDevice(), _memory);
}

void VKBuffer::allocateMemory(const VkMemoryRequirements& memReqs)
{
    if (_memory)
        vkFreeMemory(_renderer->vkLogicalDevice(), _memory, nullptr);

    _memory_allocation_info.allocationSize = memReqs.size;
    _memory_allocation_info.memoryTypeIndex = _renderer->device()->getMemoryType(memReqs.memoryTypeBits, _memory_property_flags);
    VKUtil::checkResult(vkAllocateMemory(_renderer->vkLogicalDevice(), &_memory_allocation_info, nullptr, &_memory));
}

void VKBuffer::ensureSize(const Uploader& uploader)
{
    if(_size != uploader.size())
    {
        _size = uploader.size();
        if (_descriptor.buffer)
            vkDestroyBuffer(_renderer->vkLogicalDevice(), _descriptor.buffer, nullptr);

        const VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(_usage_flags, _size);
        VKUtil::checkResult(vkCreateBuffer(_renderer->vkLogicalDevice(), &bufferCreateInfo, nullptr, &_descriptor.buffer));

        _notifier.notify();

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(_renderer->vkLogicalDevice(), _descriptor.buffer, &memReqs);
        if(_memory_allocation_info.allocationSize < memReqs.size)
            allocateMemory(memReqs);
        bind();
    }
}

void VKBuffer::bind(VkDeviceSize size, VkDeviceSize offset)
{
    vkBindBufferMemory(_renderer->vkLogicalDevice(), _descriptor.buffer, _memory, offset);
    _descriptor.offset = offset;
    _descriptor.range = size;
}

VkResult VKBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(_renderer->vkLogicalDevice(), 1, &mappedRange);
}

VkResult VKBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(_renderer->vkLogicalDevice(), 1, &mappedRange);
}

const VkDescriptorBufferInfo& VKBuffer::descriptor() const
{
    return _descriptor;
}

Notifier& VKBuffer::notifier()
{
    return _notifier;
}

}
}
