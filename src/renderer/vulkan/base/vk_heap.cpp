#include "renderer/vulkan/base/vk_heap.h"

#include "core/ark.h"
#include "core/util/log.h"

#include "renderer/base/graphics_context.h"

#include "renderer/vulkan/base/vk_device.h"

#include "app/base/application_manifest.h"

namespace ark {
namespace vulkan {

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

    if(!_heaps[typeIndex])
    {
        const ApplicationManifest& manifest = Ark::instance().manifest();
        _heaps[typeIndex] = HeapType(makeMemory(graphicsContext, manifest.heap()._device_unit_size >> 4, typeIndex), sp<HeapType::L1>::make(sizeof(VkDeviceSize) * 4));
        _heaps[typeIndex].extend(makeMemory(graphicsContext, manifest.heap()._device_unit_size, typeIndex), sp<HeapType::L2>::make());
    }

    Optional<VKMemoryPtr> opt = _heaps[typeIndex].allocate(size, alignment);
    if(!opt)
    {
        const ApplicationManifest& manifest = Ark::instance().manifest();
        CHECK(size < manifest.heap()._device_unit_size, "Out of heap memory, allocation size required: %lld, you may change device_unit_size(%d) to a greater value", size, manifest.heap()._device_unit_size);
        _heaps[typeIndex].extend(makeMemory(graphicsContext, manifest.heap()._device_unit_size, typeIndex), sp<HeapType::L2>::make());
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
    _heaps[ptr._stub->_type_index].free(ptr);
}

VKMemory VKHeap::makeMemory(GraphicsContext& graphicsContext, VkDeviceSize size, uint32_t typeIndex)
{
    VKMemory memory(_device, graphicsContext.recycler(), size, typeIndex);
    memory.upload(graphicsContext);
    LOGD("Creating memory, size: %lld, typeIndex: %d", size, typeIndex);
    return memory;
}

}
}
