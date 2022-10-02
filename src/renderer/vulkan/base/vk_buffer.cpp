#include "renderer/vulkan/base/vk_buffer.h"

#include "core/impl/writable/writable_memory.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_heap.h"
#include "renderer/vulkan/base/vk_renderer.h"
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

void VKBuffer::upload(GraphicsContext& graphicsContext)
{
    if(_uploader)
    {
        ensureSize(graphicsContext, _uploader->size());

        if(isDeviceLocal())
        {
            VKBuffer stagingBuffer(_renderer, _recycler, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            stagingBuffer.setUploader(_uploader);
            stagingBuffer.upload(graphicsContext);

            VkCommandBuffer copyCmd = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
            VkBufferCopy copyRegion = {0, 0, _size};
            vkCmdCopyBuffer(copyCmd, stagingBuffer.vkBuffer(), _descriptor.buffer, 1, &copyRegion);
            _renderer->commandPool()->flushCommandBuffer(copyCmd, true);

            stagingBuffer.recycle()(graphicsContext);
        }
        else
        {
            WritableMemory writable(_memory->map());
            _uploader->upload(writable);
            if(!isHostCoherent())
                VKUtil::checkResult(flush());
            _memory->unmap();
        }
    }
}

void VKBuffer::uploadBuffer(GraphicsContext& graphicsContext, const Buffer::Snapshot& snapshot)
{
    ensureSize(graphicsContext, snapshot._size);

    if(isDeviceLocal())
    {
        VKBuffer stagingBuffer(_renderer, _recycler, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.uploadBuffer(graphicsContext, snapshot);

        VkCommandBuffer copyCmd = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        std::vector<VkBufferCopy> copyRegions;
        for(const auto& [i, j] : snapshot._strips)
            copyRegions.push_back({i, i, j.length()});
        vkCmdCopyBuffer(copyCmd, stagingBuffer.vkBuffer(), _descriptor.buffer, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
        _renderer->commandPool()->flushCommandBuffer(copyCmd, true);

        stagingBuffer.recycle()(graphicsContext);
    }
    else
    {
        uint8_t* memory = reinterpret_cast<uint8_t*>(_memory->map());
        for(const auto& [i, j] : snapshot._strips)
            memcpy(memory + i, j.buf(), j.length());
        if(!isHostCoherent())
            VKUtil::checkResult(flush());
        _memory->unmap();
    }
}

ResourceRecycleFunc VKBuffer::recycle()
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

void VKBuffer::reload(GraphicsContext& /*graphicsContext*/, const ByteArray::Borrowed& buf)
{
    DCHECK(buf.length() <= size(), "Buffer memory overflow, buffer size: %d, source size: %d", size(), buf.length());
    DCHECK(!isDeviceLocal(), "Can't reload a Vulkan Buffer with VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT set");
    void* mapped = _memory->map();
    memcpy(mapped, buf.buf(), buf.length());
    if(!isHostCoherent())
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

void VKBuffer::ensureSize(GraphicsContext& graphicsContext, size_t size)
{
    if(_size < size)
    {
        _size = size;
        if (_descriptor.buffer)
            vkDestroyBuffer(_renderer->vkLogicalDevice(), _descriptor.buffer, nullptr);

        const VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(isDeviceLocal() ? _usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT : _usage_flags, _size);
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

bool VKBuffer::isDeviceLocal() const
{
    return _memory_property_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
}

bool VKBuffer::isHostCoherent() const
{
    return _memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
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

const VkDescriptorBufferInfo& VKBuffer::vkDescriptor() const
{
    return _descriptor;
}

Notifier& VKBuffer::notifier()
{
    return _notifier;
}

}
}
