#include "renderer/base/graphics_context.h"

#include "core/inf/variable.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"

#include "renderer/base/resource_manager.h"

#include "platform/platform.h"

namespace ark {

GraphicsContext::GraphicsContext(const sp<RenderContext>& renderContext, const sp<ResourceManager>& resourceManager)
    : _render_context(renderContext), _resource_manager(resourceManager), _tick(0)
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::onSurfaceReady()
{
    _resource_manager->onSurfaceReady(*this);
}

void GraphicsContext::onDrawFrame()
{
    ++_tick;
    _resource_manager->onDrawFrame(*this);
}

const sp<ResourceManager>& GraphicsContext::resourceManager() const
{
    return _resource_manager;
}

const sp<RenderContext>& GraphicsContext::renderContext() const
{
    return _render_context;
}

uint32_t GraphicsContext::tick() const
{
    return _tick;
}

}
