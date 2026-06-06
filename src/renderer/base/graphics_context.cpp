#include "renderer/base/graphics_context.h"

#include "render_backend.h"
#include "core/types/global.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_backend_info.h"

namespace ark {

GraphicsContext::GraphicsContext()
    : _render_backend_info(Ark::instance().renderController()->renderEngine()->context()), _render_controller(Ark::instance().renderController())
{
}

GraphicsContext::GraphicsContext(sp<RenderBackendInfo> renderBackendInfo, sp<RenderController> renderController)
    : _render_backend_info(std::move(renderBackendInfo)), _render_controller(std::move(renderController)), _tick(0)
{
}

GraphicsContext& GraphicsContext::mocked()
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    return Global<GraphicsContext>();
}

void GraphicsContext::onSurfaceReady()
{
    _render_controller->onSurfaceReady(*this);
}

void GraphicsContext::onDrawFrame()
{
    ++_tick;
    _render_controller->onDrawFrame(*this);
}

const sp<RenderController>& GraphicsContext::renderController() const
{
    return _render_controller;
}

const sp<Recycler>& GraphicsContext::recycler() const
{
    return _render_controller->recycler();
}

Traits& GraphicsContext::traits()
{
    return _render_backend_info->traits();
}

const Traits& GraphicsContext::traits() const
{
    return _render_backend_info->traits();
}

const sp<RenderBackendInfo>& GraphicsContext::renderBackendInfo() const
{
    return _render_backend_info;
}

uint32_t GraphicsContext::tick() const
{
    return _tick;
}

}
