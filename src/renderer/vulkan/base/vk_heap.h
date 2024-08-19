#pragma once

#include "core/base/heap.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vk_memory.h"
#include "renderer/vulkan/base/vk_memory_ptr.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKHeap {
private:
    typedef Heap<VKMemory, VkDeviceSize, sizeof(VkDeviceSize)> HeapType;

public:
    VKHeap(const sp<VKDevice>& device);

    VKMemoryPtr allocate(GraphicsContext& graphicsContext, const VkMemoryRequirements& memReqs, VkMemoryPropertyFlags propertyFlags);

    void recycle(GraphicsContext& graphicsContext, const VKMemoryPtr& ptr);

private:
    VKMemory makeMemory(GraphicsContext& graphicsContext, VkDeviceSize size, uint32_t typeIndex);

    VKMemoryPtr doAllocate(GraphicsContext& graphicsContext, VkDeviceSize size, VkDeviceSize alignment, uint32_t typeIndex);

private:
    sp<VKDevice> _device;

    sp<HeapType> _heaps[VK_MAX_MEMORY_TYPES];

};


}
