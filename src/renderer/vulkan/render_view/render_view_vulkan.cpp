#include "renderer/vulkan/render_view/render_view_vulkan.h"

#include <vulkan/vulkan.h>

#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/viewport.h"

#include "renderer/base/resource_manager.h"

#include "renderer/base/graphics_context.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VKUtil>& vulkanApi, const sp<VKRenderer>& renderer, const sp<GLContext>& glContext, const sp<ResourceManager>& resourceManager, const Viewport& viewport)
    : _vulkan_api(vulkanApi), _renderer(renderer), _graphics_context(new GraphicsContext(glContext, resourceManager)), _viewport(viewport)
{
}

void RenderViewVulkan::onSurfaceCreated()
{
//    _vulkan_api->initialize(_graphics_context->glContext());
}

void RenderViewVulkan::onSurfaceChanged(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d, Viewport (%.1f, %.1f, %.1f, %.1f)", width, height, _viewport.left(), _viewport.top(), _viewport.right(), _viewport.bottom());
    _graphics_context.reset(new GraphicsContext(_graphics_context->glContext(), _graphics_context->resourceManager()));

    _renderer->renderTarget()->onSurfaceChanged(width, height);
    makeCommandBuffers(_graphics_context, width, height, Color::BLACK);
}

void RenderViewVulkan::onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand)
{
/*
    _vulkan_api->render();
/*/
    const sp<VKRenderTarget>& renderTarget = _renderer->renderTarget();
    renderTarget->acquire();
    _graphics_context->onDrawFrame();
    if(backgroundColor != _background_color)
        makeCommandBuffers(_graphics_context, renderTarget->width(), renderTarget->height(), backgroundColor);
    _command_buffers->submit(_graphics_context);
    renderCommand->draw(_graphics_context);
    renderTarget->swap();
/**/
}

void RenderViewVulkan::makeCommandBuffers(GraphicsContext& graphicsContext, uint32_t width, uint32_t height, const Color& backgroundColor)
{
    _background_color = backgroundColor;

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VkClearValue clearValues[2];
    clearValues[0].color = {{_background_color.red(), _background_color.green(), _background_color.blue(), _background_color.alpha()}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = _renderer->renderTarget()->vkRenderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    _command_buffers = sp<VKCommandBuffers>::make(graphicsContext.resourceManager()->recycler(), _renderer->renderTarget());
    const std::vector<VkCommandBuffer>& commandBuffers = _command_buffers->commandBuffers();
    for (size_t i = 0; i < commandBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = _renderer->renderTarget()->frameBuffers()[i];
        VKUtil::checkResult(vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo));
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(commandBuffers[i], 0, 1, &_renderer->renderTarget()->vkViewport());
        vkCmdEndRenderPass(commandBuffers[i]);
        VKUtil::checkResult(vkEndCommandBuffer(commandBuffers[i]));
    }
}

}
}
