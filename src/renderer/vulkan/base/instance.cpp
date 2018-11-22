#include "renderer/vulkan/base/instance.h"

#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

Instance::Instance(uint32_t apiVersion)
    : _api_version(apiVersion), _extensions({VK_KHR_SURFACE_EXTENSION_NAME})
{
}

Instance::~Instance()
{
#ifdef ARK_FLAG_DEBUG
    vks::debug::freeDebugCallback(_instance);
#endif
    vkDestroyInstance(_instance, nullptr);
}

void Instance::initialize()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ark";
    appInfo.pEngineName = "ark";
    appInfo.apiVersion = VK_API_VERSION_1_0;

#if defined(_WIN32)
    _extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    _extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
    _extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    _extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    _extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    _extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    _extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &appInfo;

#ifdef ARK_FLAG_DEBUG
    _extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = _extensions.data();

#ifdef ARK_FLAG_DEBUG
        instanceCreateInfo.enabledLayerCount = vks::debug::validationLayerCount;
        instanceCreateInfo.ppEnabledLayerNames = vks::debug::validationLayerNames;
#endif

    VulkanAPI::checkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));

    uint32_t gpuCount = 0;
    VulkanAPI::checkResult(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
    DASSERT(gpuCount > 0);

    _physical_devices.resize(gpuCount);

    VulkanAPI::checkResult(vkEnumeratePhysicalDevices(_instance, &gpuCount, _physical_devices.data()));

}

VkInstance Instance::vkInstance() const
{
    return _instance;
}

const std::vector<VkPhysicalDevice>& Instance::physicalDevices() const
{
    return _physical_devices;
}

}
}
