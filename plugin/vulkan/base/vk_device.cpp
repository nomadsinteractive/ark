#include "vulkan/base/vk_device.h"

#include "vulkan/util/vulkan_tools.h"
#include "vulkan/util/vulkan_debug.h"

#include "vulkan/base/vk_command_pool.h"
#include "vulkan/base/vk_instance.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

VKDevice::VKDevice(const sp<VKInstance>& instance, const VkPhysicalDevice physicalDevice, const Enum::RendererVersion version)
    : _instance(instance), _vulkan_device(new vks::VulkanDevice(physicalDevice)), _features_vk12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES}, _features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, version >= Enum::RENDERER_VERSION_VULKAN_12 ? &_features_vk12 : nullptr},
      _enabled_features_vk12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES}, _enabled_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, version >= Enum::RENDERER_VERSION_VULKAN_12 ? &_enabled_features_vk12 : nullptr}
{
    vkGetPhysicalDeviceFeatures2(physicalDevice, &_features);
    if(_features.features.samplerAnisotropy)
        _enabled_features.features.samplerAnisotropy = VK_TRUE;
    if(_features.features.multiDrawIndirect)
        _enabled_features.features.multiDrawIndirect = VK_TRUE;
    if(_features.features.fragmentStoresAndAtomics)
        _enabled_features.features.fragmentStoresAndAtomics = VK_TRUE;
    if(_features.features.independentBlend)
        _enabled_features.features.independentBlend = VK_TRUE;

    const bool enableFeatures12 = version >= Enum::RENDERER_VERSION_VULKAN_12;
    if(enableFeatures12)
    {
        if(_features_vk12.separateDepthStencilLayouts)
            _enabled_features_vk12.separateDepthStencilLayouts = VK_TRUE;
    }

    VKUtil::checkResult(_vulkan_device->createLogicalDevice(enableFeatures12 ? nullptr : &_enabled_features.features, _enabled_extensions, enableFeatures12 ? &_enabled_features : nullptr));

    initDeviceQueue(_vulkan_device->queueFamilyIndices.graphics);
    initDeviceQueue(_vulkan_device->queueFamilyIndices.compute);

    const VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_vulkan_device->physicalDevice, &_depth_format);
    ASSERT(validDepthFormat);

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
    return _features.features;
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

uint32_t VKDevice::getMemoryType(const uint32_t typeBits, const VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
    return _vulkan_device->getMemoryType(typeBits, properties, memTypeFound);
}

VkQueue VKDevice::getQueueByFamilyIndex(uint32_t familyIndex) const
{
    return _queue_families.at(familyIndex);
}

sp<VKCommandPool> VKDevice::makeComputeCommandPool() const
{
    return sp<VKCommandPool>::make(*this, _vulkan_device->queueFamilyIndices.compute);
}

void VKDevice::createPipelineCache()
{
    constexpr VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    VKUtil::checkResult(vkCreatePipelineCache(_vulkan_device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &_pipeline_cache));
}

void VKDevice::initDeviceQueue(const uint32_t familyIndex)
{
    if(_queue_families.find(familyIndex) == _queue_families.end())
        vkGetDeviceQueue(_vulkan_device->logicalDevice, familyIndex, 0, &_queue_families[familyIndex]);
}

}