#include "renderer/vulkan/base/vk_buffer.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

VKBuffer::VKBuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, const sp<Uploader>& uploader, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
    : Buffer::Delegate(0), _renderer(renderer), _recycler(recycler), _uploader(uploader), _usage_flags(usageFlags),
      _memory_property_flags(memoryPropertyFlags), _descriptor{}, _memory_allocation_info(vks::initializers::memoryAllocateInfo())
{
}

VKBuffer::~VKBuffer()
{
    _recycler->recycle(*this);
}

uintptr_t VKBuffer::id()
{
    return static_cast<uintptr_t>(_descriptor.buffer);
}

void VKBuffer::upload(GraphicsContext& graphicsContext)
{
    reload(graphicsContext, nullptr);
}

Resource::RecycleFunc VKBuffer::recycle()
{
    const sp<VKDevice> device = _renderer->device();
    VkBuffer buffer = _descriptor.buffer;
    VkDeviceMemory memory = _memory;

    _descriptor.buffer = VK_NULL_HANDLE;
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
    const sp<Uploader>& uploader = transientUploader ? transientUploader : _uploader;
    if(uploader)
    {
        ensureSize(uploader);

        void* mapped = map();
        size_t offset = 0;
        uploader->upload([mapped, &offset](void* buf, size_t size) {
            memcpy(reinterpret_cast<int8_t*>(mapped) + offset, buf, size);
            offset += size;
        });
        if((_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            flush();
        unmap(mapped);

        if(transientUploader)
            _uploader = transientUploader;
    }
}

const VkBuffer& VKBuffer::vkBuffer() const
{
    return _descriptor.buffer;
}

void* VKBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    void* mapped = nullptr;
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

}
}
