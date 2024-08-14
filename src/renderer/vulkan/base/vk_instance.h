#pragma once

#include <vector>

#include "renderer/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKInstance {
public:
    VKInstance();
    ~VKInstance();

    void initialize(const RenderEngine& renderEngine);

    VkInstance vkInstance() const;

    const std::vector<VkPhysicalDevice>& physicalDevices() const;

private:
    void setupDebugMessageCallback();

private:
    std::vector<const char*> _extensions;

    std::vector<VkPhysicalDevice> _physical_devices;

    VkDebugUtilsMessengerEXT _callback1;
    VkDebugUtilsMessengerEXT _callback2;
    VkDebugUtilsMessengerEXT _callback3;
    VkInstance _instance;
};

}
