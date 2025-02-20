#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/vulkan_plugin.h"

using namespace ark;
using namespace ark::plugin::vulkan;

extern "C" ARK_API Plugin* __ark_vulkan_initialize__(Ark&);

Plugin* __ark_vulkan_initialize__(Ark& /*ark*/)
{
    return new VulkanPlugin();
}
