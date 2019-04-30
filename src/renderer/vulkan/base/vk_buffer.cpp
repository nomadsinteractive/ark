#include "renderer/vulkan/base/vk_buffer.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_heap.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKBuffer::VKBuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
    : _renderer(renderer), _recycler(recycler), _usage_flags(usageFlags), _memory_property_flags(memoryPropertyFlags), _descriptor{}, _memory_requirements{}
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

void VKBuffer::upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader)
{
    if(uploader)
    {
        ensureSize(graphicsContext, uploader);

        uint8_t* mapped = reinterpret_cast<uint8_t*>(_memory->map());
        uploader->upload([&mapped](void* buf, size_t size) {
            memcpy(mapped, buf, size);
            mapped += size;
        });
        if((_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            flush();
        _memory->unmap();
    }
}

Resource::RecycleFunc VKBuffer::recycle()
{
    const sp<VKDevice> device = _renderer->device();
    const sp<VKHeap> heap = _renderer->heap();
    VkBuffer buffer = _descriptor.buffer;
    VKMemoryPtr memory = std::move(_memory);

    _size = 0;
    _memory_requirements.size = 0;
    _descriptor.buffer = VK_NULL_HANDLE;

    return [device, buffer, heap, memory](GraphicsContext& graphicsContext) {
        if (buffer)
            vkDestroyBuffer(device->vkLogicalDevice(), buffer, nullptr);
        if (memory)
            heap->recycle(graphicsContext, memory);
    };
}

void VKBuffer::reload(GraphicsContext& /*graphicsContext*/, const bytearray& buf)
{
    DCHECK(buf->size() <= size(), "Buffer memory overflow, buffer size: %d, source size: %d", size(), buf->size());
    void* mapped = _memory->map();
    memcpy(mapped, buf->buf(), buf->size());
    if((_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        VKUtil::checkResult(flush());
    _memory->unmap();
}

const VkBuffer& VKBuffer::vkBuffer() const
{
    return _descriptor.buffer;
}

void VKBuffer::allocateMemory(GraphicsContext& graphicsContext, const VkMemoryRequirements& memReqs)
{
    if(_memory)
        _renderer->heap()->recycle(graphicsContext, _memory);
    _memory = _renderer->heap()->allocate(graphicsContext, memReqs, _memory_property_flags);
}

void VKBuffer::ensureSize(GraphicsContext& graphicsContext, const Uploader& uploader)
{
    if(_size < uploader.size())
    {
        _size = uploader.size();
        if (_descriptor.buffer)
            vkDestroyBuffer(_renderer->vkLogicalDevice(), _descriptor.buffer, nullptr);

        const VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(_usage_flags, _size);
        VKUtil::checkResult(vkCreateBuffer(_renderer->vkLogicalDevice(), &bufferCreateInfo, nullptr, &_descriptor.buffer));

        _notifier.notify();

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(_renderer->vkLogicalDevice(), _descriptor.buffer, &memReqs);
        if(_memory_requirements.alignment != memReqs.alignment || _memory_requirements.size != memReqs.size)
        {
            allocateMemory(graphicsContext, memReqs);
            _memory_requirements = memReqs;
        }
        bind();
    }
}

void VKBuffer::bind()
{
    vkBindBufferMemory(_renderer->vkLogicalDevice(), _descriptor.buffer, _memory.vkMemory(), _memory.offset());
    _descriptor.offset = 0;
    _descriptor.range = _size;
}

VkResult VKBuffer::flush()
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory.vkMemory();
    mappedRange.offset = 0;
    mappedRange.size = _size;
    return vkFlushMappedMemoryRanges(_renderer->vkLogicalDevice(), 1, &mappedRange);
}

VkResult VKBuffer::invalidate()
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _memory.vkMemory();
    mappedRange.offset = 0;
    mappedRange.size = _size;
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
