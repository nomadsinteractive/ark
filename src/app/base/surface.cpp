#include "app/base/surface.h"

#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_by_function.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/impl/renderer/renderer_group.h"
#include "graphics/inf/render_view.h"
#include "graphics/inf/layer.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"
#include "app/base/surface_controller.h"

#include "platform/platform.h"

namespace ark {

namespace {

class SurfaceControllerUpdateTask : public Runnable {
public:
    SurfaceControllerUpdateTask(const sp<RenderController>& synchronizer, const sp<SurfaceController>& surfaceController)
        : _synchronizer(synchronizer), _surface_controller(surfaceController) {
    }

    virtual void run() override {
        _synchronizer->preUpdate();
        _surface_controller->requestUpdate();
    }

private:
    sp<RenderController> _synchronizer;
    sp<SurfaceController> _surface_controller;
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

void Surface::onRenderFrame()
{
    _render_view->onRenderFrame(_surface_controller->getRenderCommand());
}

void Surface::scheduleUpdate(const sp<ApplicationContext>& applicationContext, uint32_t fps)
{
    applicationContext->schedule(sp<SurfaceControllerUpdateTask>::make(applicationContext->renderController(), _surface_controller), 1.0f / fps);
}

}
