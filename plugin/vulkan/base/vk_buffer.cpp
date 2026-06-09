#include "vulkan/base/vk_buffer.h"

#include "core/util/uploader_type.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/recyclable.h"

#include "vulkan/base/vk_command_pool.h"
#include "vulkan/base/vk_heap.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

namespace {

class RecyclableVKBuffer final : public Recyclable {
public:
    RecyclableVKBuffer(sp<VKDevice> device, sp<VKHeap> heap, const VkBuffer buffer, VKMemoryPtr memory)
        : _device(std::move(device)), _heap(std::move(heap)), _buffer(buffer), _memory(std::move(memory)) {
    }

    ~RecyclableVKBuffer() override {
        if(_buffer)
            vkDestroyBuffer(_device->vkLogicalDevice(), _buffer, nullptr);
        if(_memory)
            _heap->recycle(_memory);
    }

private:
    sp<VKDevice> _device;
    sp<VKHeap> _heap;
    VkBuffer _buffer;
    VKMemoryPtr _memory;
};

class RecyclableCommandBuffer final : public Recyclable {
public:
    RecyclableCommandBuffer(sp<VKCommandPool> commandPool, const VkCommandBuffer commandBuffer)
        : _command_pool(std::move(commandPool)), _command_buffer(commandBuffer)
    {
    }

    ~RecyclableCommandBuffer() override
    {
        _command_pool->destroyCommandBuffers(1, &_command_buffer);
    }

private:
    sp<VKCommandPool> _command_pool;
    VkCommandBuffer _command_buffer;
};

}

VKBuffer::VKBuffer(sp<VKRenderer> renderer, sp<Recycler> recycler, const VkBufferUsageFlags usageFlags, const VkMemoryPropertyFlags memoryPropertyFlags)
    : Delegate(), _renderer(std::move(renderer)), _recycler(std::move(recycler)), _usage_flags(usageFlags), _memory_property_flags(memoryPropertyFlags), _descriptor{}, _memory_requirements{}
{
}

VKBuffer::~VKBuffer()
{
    if(id())
        _recycler->recycle(toRecyclable());
}

uint64_t VKBuffer::id()
{
    return (uint64_t)(_descriptor.buffer);
}

void VKBuffer::upload(GraphicsContext& /*graphicsContext*/)
{
}

void VKBuffer::uploadBuffer(GraphicsContext& graphicsContext, Uploader& uploader)
{
    ensureSize(graphicsContext, uploader.size());
    if(isHostVisible())
    {
        UploaderType::writeTo(uploader, _memory->map());
        if(!isHostCoherent())
            VKUtil::checkResult(flush());
        _memory->unmap();
    }
    else if(const auto records = UploaderType::recordRanges(uploader); !records.empty())
    {
        VKBuffer stagingBuffer(_renderer, _recycler, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.uploadBuffer(graphicsContext, uploader);

        Vector<VkBufferCopy> copyRegions;
        copyRegions.reserve(records.size());
        for(const auto& [k, v] : records)
            copyRegions.push_back({k, k, v});

        sp<VKCommandPool> commandPool = _renderer->commandPool();
        const VkCommandBuffer copyCmd = commandPool->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkCmdCopyBuffer(copyCmd, stagingBuffer.vkBuffer(), _descriptor.buffer, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

        // Order the transfer write before the consuming reads on the GPU with a pipeline barrier instead of stalling
        // the CPU with a vkQueueWaitIdle. The barrier's second synchronization scope spans submission order on this
        // (graphics) queue, so it also covers the frame's render command buffer submitted later on the same queue.
        VkPipelineStageFlags dstStages = 0;
        VkAccessFlags dstAccess = 0;
        if(_usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)    { dstStages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT; dstAccess |= VK_ACCESS_INDEX_READ_BIT; }
        if(_usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)   { dstStages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT; dstAccess |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; }
        if(_usage_flags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)  { dstStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; dstAccess |= VK_ACCESS_UNIFORM_READ_BIT; }
        if(_usage_flags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)  { dstStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; dstAccess |= VK_ACCESS_SHADER_READ_BIT; }
        if(_usage_flags & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) { dstStages |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT; dstAccess |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT; }
        if(dstStages == 0) { dstStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; dstAccess = VK_ACCESS_MEMORY_READ_BIT; }

        VkBufferMemoryBarrier barrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = dstAccess;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = _descriptor.buffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStages, 0, 0, nullptr, 1, &barrier, 0, nullptr);

        commandPool->submitCommandBuffer(copyCmd);

        // No vkQueueWaitIdle: the copy is still in flight, so the command buffer cannot be freed yet and the staging
        // buffer must outlive it. Both are reclaimed by the recycler's deferred pass (hundreds of frames later, long
        // after the copy completes) instead of with a per-upload fence. The staging buffer defers via its destructor.
        _recycler->recycle(new RecyclableCommandBuffer(std::move(commandPool), copyCmd));
    }
}

void VKBuffer::downloadBuffer(GraphicsContext& graphicsContext, const size_t offset, const size_t size, void* ptr)
{
    if(isHostVisible())
    {
        memcpy(ptr, _memory->map(), size);
        _memory->unmap();
    }
    else
    {
        VKBuffer stagingBuffer(_renderer, _recycler, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.ensureSize(graphicsContext, size);

        const VkCommandBuffer copyCmd = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        const VkBufferCopy copyRegions{offset, 0, size};
        vkCmdCopyBuffer(copyCmd, _descriptor.buffer, stagingBuffer.vkBuffer(), 1, &copyRegions);
        _renderer->commandPool()->flushCommandBuffer(copyCmd, true);

        memcpy(ptr, stagingBuffer._memory->map(), size);
        stagingBuffer._memory->unmap();
        stagingBuffer.toRecyclable();
    }
}

op<Recyclable> VKBuffer::toRecyclable()
{
    op<Recyclable> recyclable(new RecyclableVKBuffer(_renderer->device(), _renderer->heap(), _descriptor.buffer, std::move(_memory)));

    _size = 0;
    _memory_requirements.size = 0;
    _descriptor.buffer = VK_NULL_HANDLE;

    return recyclable;
}

void VKBuffer::reload(GraphicsContext& /*graphicsContext*/, const ByteArray::View& buf) const
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
        _renderer->heap()->recycle(_memory);
    _memory = _renderer->heap()->allocate(graphicsContext, memReqs, _memory_property_flags);
}

void VKBuffer::ensureSize(GraphicsContext& graphicsContext, const size_t size)
{
    if(const size_t allocatingSize = std::max<size_t>(size, 16); !_memory || _memory.size() < allocatingSize)
    {
        _size = size;
        if (_descriptor.buffer)
            vkDestroyBuffer(_renderer->vkLogicalDevice(), _descriptor.buffer, nullptr);

        const VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(isDeviceLocal() ? _usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT : _usage_flags, allocatingSize);
        VKUtil::checkResult(vkCreateBuffer(_renderer->vkLogicalDevice(), &bufferCreateInfo, nullptr, &_descriptor.buffer));

        _observer.notify();

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
    _descriptor.range = _size ? _size : VK_WHOLE_SIZE;
}

VkResult VKBuffer::flush() const
{
    VkMappedMemoryRange mappedRange = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
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

bool VKBuffer::isHostVisible() const
{
    return _memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
}

VkResult VKBuffer::invalidate()
{
    VkMappedMemoryRange mappedRange = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
    mappedRange.memory = _memory.vkMemory();
    mappedRange.offset = 0;
    mappedRange.size = _size;
    return vkInvalidateMappedMemoryRanges(_renderer->vkLogicalDevice(), 1, &mappedRange);
}

const VkDescriptorBufferInfo& VKBuffer::vkDescriptor() const
{
    return _descriptor;
}

Observer& VKBuffer::observer()
{
    return _observer;
}

}
