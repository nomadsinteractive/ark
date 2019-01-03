#include "app/base/surface.h"

#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_by_function.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/impl/renderer/renderer_group.h"
#include "graphics/inf/render_view.h"
#include "graphics/base/layer.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"
#include "graphics/base/surface_controller.h"

#include "app/base/surface_updater.h"

#include "platform/platform.h"

namespace ark {

namespace {

class SurfaceControllerUpdateTask : public Runnable {
public:
    SurfaceControllerUpdateTask(const sp<RenderController>& renderController, const sp<SurfaceUpdater>& surfaceUpdater)
        : _render_controller(renderController), _surface_updater(surfaceUpdater) {
    }

    virtual void run() override {
        _render_controller->preUpdate();
        _surface_updater->requestUpdate();
    }

private:
    sp<RenderController> _render_controller;
    sp<SurfaceUpdater> _surface_updater;
};

}

Surface::Surface(const sp<RenderView>& renderView)
    : _surface_controller(sp<SurfaceController>::make()), _render_view(renderView)
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

void Surface::onSurfaceCreated()
{
    _render_view->onSurfaceCreated();
}

void Surface::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _render_view->onSurfaceChanged(width, height);
}

void Surface::onRenderFrame(const Color& backgroundColor)
{
    _surface_controller->onRenderFrame(backgroundColor, _render_view);
}

void Surface::scheduleUpdate(const sp<ApplicationContext>& applicationContext, uint32_t fps)
{
    const sp<SurfaceUpdater> surfaceUpdater = sp<SurfaceUpdater>::make(applicationContext->executor(), _surface_controller);
    applicationContext->schedule(sp<SurfaceControllerUpdateTask>::make(applicationContext->renderController(), surfaceUpdater), 1.0f / fps);
}

}
