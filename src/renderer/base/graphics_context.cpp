#include "renderer/base/graphics_context.h"

#include "render_engine.h"
#include "core/types/global.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"

namespace ark {

GraphicsContext::GraphicsContext()
    : _render_context(Ark::instance().renderController()->renderEngine()->context()), _render_controller(Ark::instance().renderController())
{
}

GraphicsContext::GraphicsContext(sp<RenderEngineContext> renderContext, sp<RenderController> renderController)
    : _render_context(std::move(renderContext)), _render_controller(std::move(renderController)), _tick(0)
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
    return _render_context->traits();
}

const Traits& GraphicsContext::traits() const
{
    return _render_context->traits();
}

const sp<RenderEngineContext>& GraphicsContext::renderContext() const
{
    return _render_context;
}

uint32_t GraphicsContext::tick() const
{
    return _tick;
}

}
