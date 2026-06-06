#include "renderer/base/graphics_context.h"

#include "render_backend.h"
#include "core/types/global.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_backend_info.h"

namespace ark {

GraphicsContext::GraphicsContext()
    : _render_controller(Ark::instance().renderController())
{
}

GraphicsContext::GraphicsContext(sp<RenderController> renderController)
    : _render_controller(std::move(renderController))
{
}

GraphicsContext& GraphicsContext::mocked()
{
    return Global<GraphicsContext>();
}

void GraphicsContext::onSurfaceReady()
{
    _render_controller->onSurfaceReady(*this);
}

void GraphicsContext::onDrawFrame()
{
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
    return _render_controller->renderBackend()->info()->traits();
}

const sp<RenderBackendInfo>& GraphicsContext::renderBackendInfo() const
{
    return _render_controller->renderBackend()->info();
}

uint32_t GraphicsContext::tick() const
{
    return _render_controller->tick();
}

}
