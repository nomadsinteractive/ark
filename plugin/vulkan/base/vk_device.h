#pragma once

#include "core/ark.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "vulkan/forwarding.h"
#include "vulkan/util/vulkan_device.hpp"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKDevice {
public:
    VKDevice(const sp<VKInstance>& instance, VkPhysicalDevice physicalDevice, Enum::RendererVersion version);
    ~VKDevice();

    VkInstance vkInstance() const;

    VkPhysicalDevice vkPhysicalDevice() const;
    VkDevice vkLogicalDevice() const;

    VkFormat vkDepthFormat() const;
    VkPipelineCache vkPipelineCache() const;

    const VkPhysicalDeviceMemoryProperties& vkMemoryProperties() const;

    void waitIdle() const;

    const VkPhysicalDeviceProperties& properties() const;
    const VkPhysicalDeviceFeatures& features() const;
    const VkPhysicalDeviceMemoryProperties& memoryProperties() const;

    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr) const;

    VkQueue getQueueByFamilyIndex(uint32_t familyIndex) const;

    sp<VKCommandPool> makeComputeCommandPool() const;

private:
    void createPipelineCache();
    void initDeviceQueue(uint32_t familyIndex);

private:
    sp<VKInstance> _instance;
    Vector<const char*> _enabled_extensions;
    op<vks::VulkanDevice> _vulkan_device;
    Map<uint32_t, VkQueue> _queue_families;
    VkFormat _depth_format;
    VkPipelineCache _pipeline_cache;

    VkPhysicalDeviceVulkan12Features _features_vk12;
    VkPhysicalDeviceFeatures2 _features;

    VkPhysicalDeviceVulkan12Features _enabled_features_vk12;
    VkPhysicalDeviceFeatures2 _enabled_features;
};

}
