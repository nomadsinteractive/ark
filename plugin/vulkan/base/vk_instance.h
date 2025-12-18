#pragma once

#include "renderer/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKInstance {
public:
    VKInstance();
    ~VKInstance();

    void initialize(const RenderEngine& renderEngine);

    VkInstance vkInstance() const;

    const Vector<VkPhysicalDevice>& physicalDevices() const;

    PipelineDescriptor* currentPipelineDescriptor() const;
    void setCurrentPipelineDescriptor(PipelineDescriptor* pipelineDescriptor);

private:
    void setupDebugMessageCallback();

private:
    Vector<const char*> _extensions;

    Vector<VkPhysicalDevice> _physical_devices;

    VkDebugUtilsMessengerEXT _callback1;
    VkInstance _instance;

    PipelineDescriptor* _current_pipeline_descriptor;
};

}
