#include "app/base/surface.h"

#include "core/ark.h"

#include "graphics/inf/render_view.h"

#include "renderer/base/render_controller.h"

#include "graphics/base/surface_controller.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

class SurfaceUpdateRequester final : public Runnable {
public:
    SurfaceUpdateRequester(sp<SurfaceController> surfaceController, const sp<ApplicationContext>& applicationContext)
        : _surface_controller(std::move(surfaceController)), _application_context(applicationContext)
    {
    }

    void run() override
    {
        DPROFILER_TRACE("SurfaceUpdate");
        DTHREAD_CHECK(THREAD_NAME_ID_CORE);
        const uint32_t tick = _application_context->onTick();
        _application_context->renderController()->onPreCompose(tick);
        _surface_controller->requestUpdate(tick);
    }

private:
    sp<SurfaceController> _surface_controller;
    sp<ApplicationContext> _application_context;
};

}

Surface::Surface(sp<RenderView> renderView, const sp<ApplicationContext>& applicationContext)
    : _render_view(std::move(renderView)), _surface_controller(sp<SurfaceController>::make()), _update_requester(sp<Runnable>::make<SurfaceUpdateRequester>(_surface_controller, applicationContext))
{
}

const sp<RenderView>& Surface::renderView() const
{
    return _render_view;
}

const sp<SurfaceController>& Surface::controller() const
{
    return _surface_controller;
}

const sp<Runnable>& Surface::updateRequester() const
{
    return _update_requester;
}

void Surface::onSurfaceCreated() const
{
    _render_view->onSurfaceCreated();
}

void Surface::onSurfaceChanged(const uint32_t width, const uint32_t height) const
{
    _render_view->onSurfaceChanged(width, height);
}

void Surface::onRenderFrame(const V4 backgroundColor) const
{
    _surface_controller->onRenderFrame(backgroundColor, _render_view);
}

}
