#include "renderer/vulkan/base/vk_renderer.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"

namespace ark {
namespace vulkan {

VKRenderer::~VKRenderer()
{
    if(_device)
        _device->waitIdle();
#if defined(_DIRECT2DISPLAY)

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    wl_shell_surface_destroy(shell_surface);
    wl_surface_destroy(surface);
    if (keyboard)
        wl_keyboard_destroy(keyboard);
    if (pointer)
        wl_pointer_destroy(pointer);
    wl_seat_destroy(seat);
    wl_shell_destroy(shell);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    // todo : android cleanup (if required)
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_destroy_window(connection, window);
    xcb_disconnect(connection);
#endif
}

const sp<VKCommandPool>& VKRenderer::commandPool() const
{
    DCHECK(_render_target, "VKRenderer uninitialized");
    return _render_target->commandPool();
}

const sp<VKDevice>& VKRenderer::device() const
{
    DCHECK(_device, "VKRenderer uninitialized");
    return _device;
}

const sp<VKHeap>& VKRenderer::heap() const
{
    DCHECK(_heap, "VKRenderer uninitialized");
    return _heap;
}

const sp<VKRenderTarget>& VKRenderer::renderTarget() const
{
    DCHECK(_render_target, "VKRenderer uninitialized");
    return _render_target;
}

VkDevice VKRenderer::vkLogicalDevice() const
{
    DCHECK(_device, "VKRenderer uninitialized");
    return _device->vkLogicalDevice();
}

VkPhysicalDevice VKRenderer::vkPhysicalDevice() const
{
    DCHECK(_device, "VKRenderer uninitialized");
    return _device->vkPhysicalDevice();
}

VkRenderPass VKRenderer::vkRenderPass() const
{
    DCHECK(_render_target, "VKRenderer uninitialized");
    return _render_target->vkRenderPass();
}

}
}
