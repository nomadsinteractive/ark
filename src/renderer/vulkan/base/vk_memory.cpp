#include "renderer/vulkan/base/vk_memory.h"

#include "renderer/base/recycler.h"
#include "renderer/base/resource_manager.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_memory_ptr.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/util/vk_util.h"
#include "renderer/vulkan/util/vulkan_initializers.hpp"

namespace ark {
namespace vulkan {

VKMemory::Stub::Stub(const sp<VKDevice>& device, const sp<ResourceManager>& resourceManager, VkDeviceSize size, uint32_t memoryType)
    : _device(device), _resource_manager(resourceManager), _memory(VK_NULL_HANDLE), _allocation_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, size, memoryType}
{
}

VKMemory::Stub::~Stub()
{
    _resource_manager->recycler()->recycle(*this);
}

uint64_t VKMemory::Stub::id()
{
    return (uint64_t)(_memory);
}

void VKMemory::Stub::upload(GraphicsContext& /*graphicsContext*/, const sp<Uploader>& /*uploader*/)
{
    VKUtil::checkResult(vkAllocateMemory(_device->vkLogicalDevice(), &_allocation_info, nullptr, &_memory));
}

Resource::RecycleFunc VKMemory::Stub::recycle()
{
    const sp<VKDevice> device = _device;
    VkDeviceMemory memory = _memory;
    _memory = VK_NULL_HANDLE;

    return [device, memory](GraphicsContext& ) {
        if(memory)
            vkFreeMemory(device->vkLogicalDevice(), memory, nullptr);
    };
}

VKMemory::VKMemory(const sp<VKDevice>& device, const sp<ResourceManager>& resourceManager, VkDeviceSize size, uint32_t memoryType)
    : _stub(sp<Stub>::make(device, resourceManager, size, memoryType))
{
}

VKMemory::VKMemory(const sp<VKMemory::Stub>& stub)
    : _stub(stub)
{
}

void VKMemory::upload(GraphicsContext& graphicsContext)
{
    _stub->upload(graphicsContext, nullptr);
}

void* VKMemory::map(VkDeviceSize offset, VkDeviceSize size)
{
    void* mapped = nullptr;
    DCHECK(_stub->_memory, "Mapping to NULL memory");
    VKUtil::checkResult(vkMapMemory(_stub->_device->vkLogicalDevice(), _stub->_memory, offset, size, 0, &mapped));
    return mapped;
}

void VKMemory::unmap()
{
    vkUnmapMemory(_stub->_device->vkLogicalDevice(), _stub->_memory);
}

VkDeviceMemory VKMemory::vkMemory() const
{
    return _stub->_memory;
}

VkDeviceSize VKMemory::size() const
{
    return _stub->_allocation_info.allocationSize;
}

VKMemoryPtr VKMemory::begin() const
{
    return VKMemoryPtr(_stub, 0, 0);
}

VKMemoryPtr VKMemory::end() const
{
    return VKMemoryPtr(_stub, size(), 0);
}

}
}
