#include "renderer/vulkan/base/vk_instance.h"

#include "core/ark.h"
#include "core/util/log.h"

#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "renderer/vulkan/util/vk_util.h"

#include "app/base/application_manifest.h"

#if defined(ARK_FLAG_DEBUG) && !defined(ARK_PLATFORM_DARWIN) && !defined(ARK_PLATFORM_ANDROID)
#define ARK_VK_DEBUG_LAYER_ENABLED  1
#else
#define ARK_VK_DEBUG_LAYER_ENABLED  0
#endif

namespace ark::vulkan {

namespace {

VkBool32 vkDebugUtilsMessengerCallbackEXT(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData) {
    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        LOGE("[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOGW("[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    else
        LOGD("[%s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    return VK_FALSE;
}

std::vector<std::string> get_supported_extensions() {
    uint32_t count = 0;
    VKUtil::checkResult(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> extensionProperties(count);

    // Get the extensions
    VKUtil::checkResult(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data()));

    std::vector<std::string> extensions;
    for(auto& extension : extensionProperties)
        extensions.push_back(extension.extensionName);

    return extensions;
}

std::vector<VkLayerProperties> getInstanceLayerProperties() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return availableLayers;
}

}


VKInstance::VKInstance(uint32_t apiVersion)
    : _api_version(apiVersion), _extensions({VK_KHR_SURFACE_EXTENSION_NAME}), _callback1(VK_NULL_HANDLE), _callback2(VK_NULL_HANDLE), _callback3(VK_NULL_HANDLE)
{
}

VKInstance::~VKInstance()
{
#ifdef ARK_FLAG_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    if(_callback1)
        pfnDestroyDebugUtilsMessengerEXT(_instance, _callback1, nullptr);
    if(_callback2)
        pfnDestroyDebugUtilsMessengerEXT(_instance, _callback2, nullptr);
    if(_callback3)
        pfnDestroyDebugUtilsMessengerEXT(_instance, _callback3, nullptr);
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

    VkInstanceCreateInfo instanceCreateInfo = {};
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
    _extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &appInfo;

#if ARK_VK_DEBUG_LAYER_ENABLED
    std::vector<const char*> validationLayerNames;
    const std::vector<VkLayerProperties> layerProperties = getInstanceLayerProperties();
    for(const auto& i : layerProperties)
        if(strcmp(i.layerName, "VK_LAYER_KHRONOS_validation") == 0)
        {
            validationLayerNames.push_back(i.layerName);
            break;
        }

    _extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    CHECK_WARN(validationLayerNames.size() == 1, "VK_LAYER_KHRONOS_validation not found");
    instanceCreateInfo.enabledLayerCount = validationLayerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
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
    setupDebugMessageCallback();
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

void VKInstance::setupDebugMessageCallback()
{
    // Must call extension functions through a function pointer:
    PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerCreateInfoEXT callback1 = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,  // sType
        nullptr,                                                  // pNext
        0,                                                        // flags
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |           // messageSeverity
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |             // messageType
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        vkDebugUtilsMessengerCallbackEXT,                         // pfnUserCallback
        nullptr                                                   // pUserData
    };
    VKUtil::checkResult(pfnCreateDebugUtilsMessengerEXT(_instance, &callback1, nullptr, &_callback1));

//    callback1.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//    callback1.pfnUserCallback = myDebugBreak;
//    callback1.pUserData = NULL;
//    VKUtil::checkResult(pfnCreateDebugUtilsMessengerEXT(instance, &callback1, NULL, &_callback2));

//    VkDebugUtilsMessengerCreateInfoEXT callback3 = {
//        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,  // sType
//        nullptr,                                                  // pNext
//        0,                                                        // flags
//        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,          // messageSeverity
//        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |             // messageType
//        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
//        mystdOutLogger,                                           // pfnUserCallback
//        nullptr                                                   // pUserData
//    };

//    VKUtil::checkResult(pfnCreateDebugUtilsMessengerEXT(instance, &callback3, NULL, &_callback3));
}

}
