#include "vulkan/impl/render_view/render_view_vulkan.h"

#include "core/base/future.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/render_controller.h"

#include "vulkan/base/vk_command_buffers.h"
#include "vulkan/base/vk_compute_context.h"
#include "vulkan/base/vk_device.h"
#include "vulkan/base/vk_graphics_context.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/base/vk_swap_chain.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VKRenderer>& renderer, sp<RenderController> renderController)
    : _renderer(renderer), _render_controller(std::move(renderController))
{
}

void RenderViewVulkan::onSurfaceCreated()
{
}

void RenderViewVulkan::onSurfaceChanged(const uint32_t width, const uint32_t height)
{
    LOGD("Width: %d, Height: %d", width, height);

    // Frames may still be in flight from the previous surface; drain the device before the swapchain, its
    // framebuffers and the per-frame synchronization objects are torn down and recreated below.
    if(_vk_graphics_context)
        _renderer->device()->waitIdle();

    _renderer->renderTarget()->onSurfaceChanged(width, height);


    GraphicsContext graphicsContext(_render_controller);
    _vk_graphics_context = sp<VKGraphicsContext>::make(_render_controller, _renderer);
    graphicsContext.traits().put<VKGraphicsContext>(_vk_graphics_context);

    _vk_compute_context = sp<VKComputeContext>::make(_renderer);
    graphicsContext.traits().put<VKComputeContext>(_vk_compute_context);

    graphicsContext.onSurfaceReady();
}

void RenderViewVulkan::onRenderFrame(const V4& backgroundColor, RenderCommand& renderCommand)
{
    GraphicsContext graphicsContext(_render_controller);
    _vk_graphics_context->waitForFrameAvailable();
    graphicsContext.onDrawFrame();

    const sp<VKSwapChain>& swapChain = _renderer->renderTarget();
    const uint32_t imageId = swapChain->acquire(_vk_graphics_context);

    _vk_graphics_context->begin(imageId, backgroundColor);
    renderCommand.draw(graphicsContext);
    _vk_graphics_context->end();

    swapChain->swap(_vk_graphics_context);

    if(_vk_compute_context->vkCommandBuffer() != VK_NULL_HANDLE)
        _vk_compute_context->end();
}

sp<Bitmap> RenderViewVulkan::doScreenshot()
{
    return _renderer->renderTarget()->screenshot();
}

}
