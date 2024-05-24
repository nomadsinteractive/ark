#pragma once

#include <map>
#include <vector>

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/util/vulkan_device.hpp"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKDevice {
public:
    VKDevice(const sp<VKInstance>& instance, VkPhysicalDevice vkPhysicalDevice);
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

    std::vector<const char*> _enabled_extensions;

    op<vks::VulkanDevice> _vulkan_device;

    std::map<uint32_t, VkQueue> _queue_families;
    VkFormat _depth_format;
    VkPipelineCache _pipeline_cache;
};

}
