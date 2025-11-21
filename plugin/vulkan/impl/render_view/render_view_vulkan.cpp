#include "vulkan/impl/render_view/render_view_vulkan.h"

#include "core/util/log.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/render_controller.h"

#include "vulkan/base/vk_command_buffers.h"
#include "vulkan/base/vk_compute_context.h"
#include "vulkan/base/vk_graphics_context.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/base/vk_swap_chain.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VKRenderer>& renderer, const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
    : _renderer(renderer), _graphics_context(new GraphicsContext(renderContext, renderController))
{
}

void RenderViewVulkan::onSurfaceCreated()
{
}

void RenderViewVulkan::onSurfaceChanged(const uint32_t width, const uint32_t height)
{
    LOGD("Width: %d, Height: %d", width, height);
    _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));

    _renderer->renderTarget()->onSurfaceChanged(width, height);

    _vk_graphics_context = sp<VKGraphicsContext>::make(_graphics_context, _renderer);
    _graphics_context->traits().put<VKGraphicsContext>(_vk_graphics_context);

    _vk_compute_context = sp<VKComputeContext>::make(_graphics_context, _renderer);
    _graphics_context->traits().put<VKComputeContext>(_vk_compute_context);

    _graphics_context->onSurfaceReady();
}

void RenderViewVulkan::onRenderFrame(const V4& backgroundColor, RenderCommand& renderCommand)
{
    _graphics_context->onDrawFrame();

    const sp<VKSwapChain>& swapChain = _renderer->renderTarget();
    const uint32_t imageId = swapChain->acquire(_vk_graphics_context);

    _vk_graphics_context->begin(imageId, backgroundColor);
    renderCommand.draw(_graphics_context);
    _vk_graphics_context->end();

    swapChain->swap(_vk_graphics_context);

    if(_vk_compute_context->vkCommandBuffer() != VK_NULL_HANDLE)
        _vk_compute_context->end();

    swapChain->waitIdle();
}

}
