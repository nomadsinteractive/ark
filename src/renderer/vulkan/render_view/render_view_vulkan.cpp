#include "renderer/vulkan/render_view/render_view_vulkan.h"

#include <vulkan/vulkan.h>

#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/viewport.h"

#include "renderer/base/graphics_context.h"
#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VulkanAPI>& vulkanApi, const sp<GLContext>& glContext, const sp<GLResourceManager>& glResourceManager, const Viewport& viewport)
    : _vulkan_api(vulkanApi), _graphics_context(new GraphicsContext(glContext, glResourceManager)), _viewport(viewport)
{
}

void RenderViewVulkan::onSurfaceCreated()
{
    _vulkan_api->initialize(_graphics_context->glContext());
}

void RenderViewVulkan::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _graphics_context.reset(new GraphicsContext(_graphics_context->glContext(), _graphics_context->glResourceManager()));
    initialize(width, height);
}

void RenderViewVulkan::onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand)
{
    _vulkan_api->render();
}

void RenderViewVulkan::initialize(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d, Viewport (%.1f, %.1f, %.1f, %.1f)", width, height, _viewport.left(), _viewport.top(), _viewport.right(), _viewport.bottom());
}

}
}