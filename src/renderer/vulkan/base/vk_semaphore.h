#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKSemaphore {
public:
    VKSemaphore(sp<VKRenderer> renderer, VkSemaphore semaphore = VK_NULL_HANDLE);
    ~VKSemaphore();

    VkSemaphore vkSemaphore() const;

private:
    sp<VKRenderer> _renderer;

    VkSemaphore _semaphore;
};

}
