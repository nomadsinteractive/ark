#include "renderer/base/graphics_context.h"

#include "core/inf/variable.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"

#include "renderer/base/resource_manager.h"

#include "platform/platform.h"

namespace ark {

GraphicsContext::GraphicsContext(const sp<GLContext>& glContext, const sp<ResourceManager>& resourceManager)
    : _gl_context(glContext), _resource_manager(resourceManager), _steady_clock(Platform::getSteadyClock())
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
    _tick = _steady_clock->val();
    _resource_manager->onDrawFrame(*this);
}

const sp<ResourceManager>& GraphicsContext::resourceManager() const
{
    return _resource_manager;
}

const sp<GLContext>& GraphicsContext::glContext() const
{
    return _gl_context;
}

uint64_t GraphicsContext::tick() const
{
    return _tick;
}

}
