#include "renderer/vulkan/base/vk_device.h"

#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKDevice::VKDevice(const sp<VKInstance>& instance, VkPhysicalDevice physicalDevice)
    : _instance(instance), _vulkan_device(new vks::VulkanDevice(physicalDevice))
{
    _vulkan_device->enabledFeatures = {};
    if (_vulkan_device->features.samplerAnisotropy) {
        _vulkan_device->enabledFeatures.samplerAnisotropy = VK_TRUE;
    };

    VKUtil::checkResult(_vulkan_device->createLogicalDevice(_vulkan_device->enabledFeatures, _enabled_extensions));

    // Get a graphics queue from the device
    vkGetDeviceQueue(_vulkan_device->logicalDevice, _vulkan_device->queueFamilyIndices.graphics, 0, &_queue);

    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_vulkan_device->physicalDevice, &_depth_format);
    DASSERT(validDepthFormat);

    createPipelineCache();
}

VKDevice::~VKDevice()
{
    vkDestroyPipelineCache(_vulkan_device->logicalDevice, _pipeline_cache, nullptr);
}

VkInstance VKDevice::vkInstance() const
{
    return _instance->vkInstance();
}

VkPhysicalDevice VKDevice::vkPhysicalDevice() const
{
    return _vulkan_device->physicalDevice;
}

VkDevice VKDevice::vkLogicalDevice() const
{
    return _vulkan_device->logicalDevice;
}

VkQueue VKDevice::vkQueue() const
{
    return _queue;
}

void VKDevice::waitIdle() const
{
    vkDeviceWaitIdle(_vulkan_device->logicalDevice);
}

const VkPhysicalDeviceProperties& VKDevice::properties() const
{
    return _vulkan_device->properties;
}

const VkPhysicalDeviceFeatures& VKDevice::features() const
{
    return _vulkan_device->features;
}

const VkPhysicalDeviceMemoryProperties& VKDevice::memoryProperties() const
{
    return _vulkan_device->memoryProperties;
}

VkFormat VKDevice::vkDepthFormat() const
{
    return _depth_format;
}

VkPipelineCache VKDevice::vkPipelineCache() const
{
    return _pipeline_cache;
}

const VkPhysicalDeviceMemoryProperties& VKDevice::vkMemoryProperties() const
{
    return _vulkan_device->memoryProperties;
}

uint32_t VKDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
    return _vulkan_device->getMemoryType(typeBits, properties, memTypeFound);
}

void VKDevice::createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VKUtil::checkResult(vkCreatePipelineCache(_vulkan_device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &_pipeline_cache));
}

}
}
