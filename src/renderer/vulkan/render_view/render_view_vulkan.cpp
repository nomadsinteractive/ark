#include "renderer/vulkan/render_view/render_view_vulkan.h"

#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/viewport.h"

#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_compute_context.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark::vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VKRenderer>& renderer, const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
    : _renderer(renderer), _graphics_context(new GraphicsContext(renderContext, renderController))
{
}

void RenderViewVulkan::onSurfaceCreated()
{
}

void RenderViewVulkan::onSurfaceChanged(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d", width, height);
    _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));

    _renderer->renderTarget()->onSurfaceChanged(width, height);

    _vk_graphics_context = sp<VKGraphicsContext>::make(_graphics_context, _renderer);
    _graphics_context->attachments().put<VKGraphicsContext>(_vk_graphics_context);

    _vk_compute_context = sp<VKComputeContext>::make(_graphics_context, _renderer);
    _graphics_context->attachments().put<VKComputeContext>(_vk_compute_context);

    _graphics_context->onSurfaceReady();
}

void RenderViewVulkan::onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand)
{
    _graphics_context->onDrawFrame();

    const sp<VKRenderTarget>& renderTarget = _renderer->renderTarget();
    uint32_t imageId = renderTarget->acquire(_vk_graphics_context);

    _vk_graphics_context->begin(imageId, backgroundColor);
    renderCommand.draw(_graphics_context);
    _vk_graphics_context->end();

    renderTarget->swap(_vk_graphics_context);

    if(_vk_compute_context->vkCommandBuffer() != VK_NULL_HANDLE)
    {
        _vk_compute_context->end();
        _vk_compute_context->submit();
        _vk_graphics_context->addWaitSemaphore(_vk_compute_context->semaphoreComputeComplete(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
}

}
