#include "renderer/vulkan/render_view/render_view_vulkan.h"

#include <vulkan/vulkan.h>

#include "core/util/log.h"

#include "graphics/inf/render_command.h"
#include "graphics/base/viewport.h"

#include "renderer/base/graphics_context.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

RenderViewVulkan::RenderViewVulkan(const sp<VKUtil>& vulkanApi, const sp<GLContext>& glContext, const sp<ResourceManager>& resourceManager, const Viewport& viewport)
    : _vulkan_api(vulkanApi), _graphics_context(new GraphicsContext(glContext, resourceManager)), _viewport(viewport)
{
}

void RenderViewVulkan::onSurfaceCreated()
{
    _vulkan_api->initialize(_graphics_context->glContext());
}

void RenderViewVulkan::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _graphics_context.reset(new GraphicsContext(_graphics_context->glContext(), _graphics_context->resourceManager()));
    initialize(width, height);
}

void RenderViewVulkan::onRenderFrame(const Color& backgroundColor, const sp<RenderCommand>& renderCommand)
{
//*
    _vulkan_api->render();
/*/
    _graphics_context->onDrawFrame();
    renderCommand->draw(_graphics_context);
/**/
}

void RenderViewVulkan::initialize(uint32_t width, uint32_t height)
{
    LOGD("Width: %d, Height: %d, Viewport (%.1f, %.1f, %.1f, %.1f)", width, height, _viewport.left(), _viewport.top(), _viewport.right(), _viewport.bottom());
}

}
}
