#ifndef ARK_RENDERER_VULKAN_BASE_VK_DEVICE_H_
#define ARK_RENDERER_VULKAN_BASE_VK_DEVICE_H_

#include <vector>

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/util/vulkan_device.hpp"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKDevice {
public:
    VKDevice(const sp<VKInstance>& instance, VkPhysicalDevice vkPhysicalDevice);
    ~VKDevice();

    VkInstance vkInstance() const;

    VkPhysicalDevice vkPhysicalDevice() const;
    VkDevice vkLogicalDevice() const;
    VkQueue vkQueue() const;

    VkFormat vkDepthFormat() const;
    VkPipelineCache vkPipelineCache() const;

    void waitIdle() const;

    const VkPhysicalDeviceProperties& properties() const;
    const VkPhysicalDeviceFeatures& features() const;
    const VkPhysicalDeviceMemoryProperties& memoryProperties() const;

    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);

private:
    void createPipelineCache();

private:
    sp<VKInstance> _instance;

    std::vector<const char*> _enabled_extensions;

    op<vks::VulkanDevice> _vulkan_device;

    VkQueue _queue;
    VkFormat _depth_format;
    VkPipelineCache _pipeline_cache;
};

}
}

#endif
