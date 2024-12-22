#include "renderer/base/graphics_context.h"

#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/util/strings.h"

#include "renderer/base/render_controller.h"

#include "platform/platform.h"

namespace ark {

GraphicsContext::GraphicsContext(sp<RenderEngineContext> renderContext, sp<RenderController> renderController)
    : _render_context(std::move(renderContext)), _render_controller(std::move(renderController)), _tick(0)
{
}

GraphicsContext& GraphicsContext::mocked()
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
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

Traits& GraphicsContext::attachments()
{
    return _attachments;
}

const Traits& GraphicsContext::attachments() const
{
    return _attachments;
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
