#include "vulkan/base/vk_semaphore.h"

#include "vulkan/base/vk_renderer.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

VKSemaphore::VKSemaphore(sp<VKRenderer> renderer, VkSemaphore semaphore)
    : _renderer(std::move(renderer)), _semaphore(semaphore)
{
    if(_semaphore == VK_NULL_HANDLE)
    {
        constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VKUtil::checkResult(vkCreateSemaphore(_renderer->vkLogicalDevice(), &semaphoreCreateInfo, nullptr, &_semaphore));
    }
}

VKSemaphore::~VKSemaphore()
{
    vkDestroySemaphore(_renderer->vkLogicalDevice(), _semaphore, nullptr);
}

VkSemaphore VKSemaphore::vkSemaphore() const
{
    return _semaphore;
}

}
