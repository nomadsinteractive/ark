#include "renderer/vulkan/base/vk_instance.h"

#include "core/ark.h"
#include "core/util/log.h"

#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "renderer/vulkan/util/vk_util.h"

#include "app/base/application_manifest.h"

#if defined(ARK_FLAG_DEBUG) && !defined(ARK_PLATFORM_DARWIN) && !defined(ARK_PLATFORM_ANDROID) && 0
#define ARK_VK_DEBUG_LAYER_ENABLED  1
#else
#define ARK_VK_DEBUG_LAYER_ENABLED  0
#endif

namespace ark {
namespace vulkan {

VKInstance::VKInstance(uint32_t apiVersion)
    : _api_version(apiVersion), _extensions({VK_KHR_SURFACE_EXTENSION_NAME})
{
}

VKInstance::~VKInstance()
{
#ifdef ARK_FLAG_DEBUG
    vks::debug::freeDebugCallback(_instance);
#endif
    vkDestroyInstance(_instance, nullptr);
}

void VKInstance::initialize()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Ark::instance().manifest()->name().c_str();
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

#if ARK_VK_DEBUG_LAYER_ENABLED
    _extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    instanceCreateInfo.enabledLayerCount = vks::debug::validationLayerCount;
    instanceCreateInfo.ppEnabledLayerNames = vks::debug::validationLayerNames;
#endif

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = _extensions.data();

    VKUtil::checkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));

    uint32_t gpuCount = 0;
    VKUtil::checkResult(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
    DASSERT(gpuCount > 0);

    _physical_devices.resize(gpuCount);

    VKUtil::checkResult(vkEnumeratePhysicalDevices(_instance, &gpuCount, _physical_devices.data()));

#if ARK_VK_DEBUG_LAYER_ENABLED
    vks::debug::setupDebugging(_instance, VK_DEBUG_REPORT_ERROR_BIT_EXT |
                               VK_DEBUG_REPORT_WARNING_BIT_EXT |
                               VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_NULL_HANDLE);
#endif
}

VkInstance VKInstance::vkInstance() const
{
    return _instance;
}

const std::vector<VkPhysicalDevice>& VKInstance::physicalDevices() const
{
    return _physical_devices;
}

}
}
