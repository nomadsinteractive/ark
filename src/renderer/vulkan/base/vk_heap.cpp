#include "renderer/vulkan/base/vk_heap.h"

#include "core/ark.h"
#include "core/util/log.h"

#include "renderer/base/graphics_context.h"

#include "renderer/vulkan/base/vk_device.h"

#include "app/base/application_manifest.h"

namespace ark::vulkan {

VKHeap::VKHeap(const sp<VKDevice>& device)
    : _device(device)
{
}

VKMemoryPtr VKHeap::allocate(GraphicsContext& graphicsContext, const VkMemoryRequirements& memReqs, VkMemoryPropertyFlags propertyFlags)
{
    uint32_t typeIndex = _device->getMemoryType(memReqs.memoryTypeBits, propertyFlags);
    return doAllocate(graphicsContext, memReqs.size, memReqs.alignment, typeIndex);
}

VKMemoryPtr VKHeap::doAllocate(GraphicsContext& graphicsContext, VkDeviceSize size, VkDeviceSize alignment, uint32_t typeIndex)
{
    DASSERT(typeIndex < VK_MAX_MEMORY_TYPES);

    sp<HeapType>& heap = _heaps[typeIndex];
    if(!heap)
    {
        const ApplicationManifest& manifest = Ark::instance().manifest();
        heap = sp<HeapType>::make(makeMemory(graphicsContext, manifest.heap()._device_unit_size, typeIndex));
    }

    Optional<VKMemoryPtr> opt = heap->allocate(size, alignment);
    if(!opt)
    {
        const ApplicationManifest& manifest = Ark::instance().manifest();
        CHECK(size < manifest.heap()._device_unit_size, "Out of heap memory, allocation size required: %lld, you may change device_unit_size(%d) to a greater value", size, manifest.heap()._device_unit_size);
        heap->extend(sp<HeapType>::make(makeMemory(graphicsContext, manifest.heap()._device_unit_size, typeIndex)));
        return doAllocate(graphicsContext, size, alignment, typeIndex);
    }

    VKMemoryPtr memory = std::move(opt.value());
    CHECK(memory.offset() % alignment == 0, "Alignment(%d) unsatisfied, size: %d, offset: %d", alignment, size, memory.offset());

    memory._stub->_size = size;
    memory._stub->_type_index = typeIndex;
    return memory;
}

void VKHeap::recycle(GraphicsContext& /*graphicsContext*/, const VKMemoryPtr& ptr)
{
    DASSERT(ptr._stub->_type_index < VK_MAX_MEMORY_TYPES);
    _heaps[ptr._stub->_type_index]->free(ptr);
}

VKMemory VKHeap::makeMemory(GraphicsContext& graphicsContext, VkDeviceSize size, uint32_t typeIndex)
{
    VKMemory memory(_device, graphicsContext.recycler(), size, typeIndex);
    memory.upload(graphicsContext);
    LOGD("Creating memory, size: %lld, typeIndex: %d", size, typeIndex);
    return memory;
}

}
