#include "renderer/vulkan/base/vk_memory.h"

#include "renderer/base/recycler.h"
#include "renderer/base/resource_manager.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/util/vk_util.h"
#include "renderer/vulkan/util/vulkan_initializers.hpp"

namespace ark {
namespace vulkan {

VKMemory::VKMemory(const sp<VKRenderer>& renderer, VkDeviceSize size, uint32_t memoryType)
    : _renderer(renderer), _memory(VK_NULL_HANDLE), _allocation_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, size, memoryType}
{
}

VKMemory::~VKMemory()
{
    _renderer->resourceManager()->recycler()->recycle(*this);
}

VkDeviceSize VKMemory::size() const
{
    return _allocation_info.allocationSize;
}

uint64_t VKMemory::id()
{
    return (uint64_t)(_memory);
}

void VKMemory::upload(GraphicsContext& /*graphicsContext*/, const sp<Uploader>& /*uploader*/)
{
    VKUtil::checkResult(vkAllocateMemory(_renderer->vkLogicalDevice(), &_allocation_info, nullptr, &_memory));
}

Resource::RecycleFunc VKMemory::recycle()
{
    const sp<VKDevice> device = _renderer->device();
    VkDeviceMemory memory = _memory;
    _memory = VK_NULL_HANDLE;

    return [device, memory](GraphicsContext& ) {
        if(memory)
            vkFreeMemory(device->vkLogicalDevice(), memory, nullptr);
    };
}

void* VKMemory::map(VkDeviceSize size, VkDeviceSize offset)
{
    void* mapped = nullptr;
    DCHECK(_memory, "Mapping to NULL memory");
    VKUtil::checkResult(vkMapMemory(_renderer->vkLogicalDevice(), _memory, offset, size, 0, &mapped));
    return mapped;
}

void VKMemory::unmap()
{
    vkUnmapMemory(_renderer->vkLogicalDevice(), _memory);
}

VkDeviceMemory VKMemory::vkMemory() const
{
    return _memory;
}

}
}
