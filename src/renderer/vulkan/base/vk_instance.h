#ifndef ARK_RENDERER_VULKAN_BASE_VK_INSTANCE_H_
#define ARK_RENDERER_VULKAN_BASE_VK_INSTANCE_H_

#include <vector>

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKInstance {
public:
    VKInstance(uint32_t apiVersion = VK_API_VERSION_1_0);
    ~VKInstance();

    void initialize();

    VkInstance vkInstance() const;

    const std::vector<VkPhysicalDevice>& physicalDevices() const;

private:
    void setupDebugMessageCallback();

private:
    uint32_t _api_version;
    std::vector<const char*> _extensions;

    std::vector<VkPhysicalDevice> _physical_devices;

    VkDebugUtilsMessengerEXT _callback1;
    VkDebugUtilsMessengerEXT _callback2;
    VkDebugUtilsMessengerEXT _callback3;
    VkInstance _instance;
};

}
}

#endif
