#ifndef _PLATFORM_VULKAN_H
#define _PLATFORM_VULKAN_H

#include "core/forwarding.h"

#include "platform/vulkan/_vulkan_impl.h"

namespace ark {

const VkIndexType kVKIndexType = sizeof(element_index_t) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

}

#endif
