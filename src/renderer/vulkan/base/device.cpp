#include "renderer/vulkan/base/device.h"

#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "renderer/vulkan/base/instance.h"
#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

Device::Device(const sp<Instance>& instance, VkPhysicalDevice physicalDevice)
    : _instance(instance), _vulkan_device(new vks::VulkanDevice(physicalDevice))
{
    _vulkan_device->enabledFeatures = {};
    if (_vulkan_device->features.samplerAnisotropy) {
        _vulkan_device->enabledFeatures.samplerAnisotropy = VK_TRUE;
    };

    VulkanAPI::checkResult(_vulkan_device->createLogicalDevice(_vulkan_device->enabledFeatures, _enabled_extensions));

    // Get a graphics queue from the device
    vkGetDeviceQueue(_vulkan_device->logicalDevice, _vulkan_device->queueFamilyIndices.graphics, 0, &_queue);

    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_vulkan_device->physicalDevice, &_depth_format);
    DASSERT(validDepthFormat);

    createPipelineCache();
}

Device::~Device()
{
    vkDestroyPipelineCache(_vulkan_device->logicalDevice, _pipeline_cache, nullptr);
}

VkInstance Device::vkInstance() const
{
    return _instance->vkInstance();
}

VkPhysicalDevice Device::physicalDevice() const
{
    return _vulkan_device->physicalDevice;
}

VkDevice Device::logicalDevice() const
{
    return _vulkan_device->logicalDevice;
}

VkQueue Device::queue() const
{
    return _queue;
}

void Device::waitIdle() const
{
    vkDeviceWaitIdle(_vulkan_device->logicalDevice);
}

const VkPhysicalDeviceProperties& Device::properties() const
{
    return _vulkan_device->properties;
}

const VkPhysicalDeviceFeatures& Device::features() const
{
    return _vulkan_device->features;
}

const VkPhysicalDeviceMemoryProperties& Device::memoryProperties() const
{
    return _vulkan_device->memoryProperties;
}

VkFormat Device::depthFormat() const
{
    return _depth_format;
}

VkPipelineCache Device::pipelineCache() const
{
    return _pipeline_cache;
}

uint32_t Device::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
    return _vulkan_device->getMemoryType(typeBits, properties, memTypeFound);
}

VkResult Device::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data) const
{
    return _vulkan_device->createBuffer(usageFlags, memoryPropertyFlags, size, buffer, memory, data);
}

VkResult Device::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vks::Buffer* buffer, VkDeviceSize size, void* data) const
{
    return _vulkan_device->createBuffer(usageFlags, memoryPropertyFlags, buffer, size, data);
}

void Device::createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VulkanAPI::checkResult(vkCreatePipelineCache(_vulkan_device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &_pipeline_cache));
}

}
}
