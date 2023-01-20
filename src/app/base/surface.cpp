#include "app/base/surface.h"

#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_by_function.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/impl/renderer/render_group.h"
#include "graphics/inf/render_view.h"
#include "graphics/base/render_layer.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"
#include "graphics/base/surface_controller.h"

#include "app/base/surface_updater.h"

namespace ark {

Surface::Surface(sp<RenderView> renderView, const ApplicationContext& applicationContext)
    : _render_view(std::move(renderView)), _surface_controller(sp<SurfaceController>::make(applicationContext.executorPooled())),
      _updater(sp<SurfaceUpdater>::make(_surface_controller, applicationContext))
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

const sp<SurfaceUpdater>& Surface::updater() const
{
    return _updater;
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

}
