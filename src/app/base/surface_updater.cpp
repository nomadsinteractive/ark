#include "app/base/surface_updater.h"

#include "core/ark.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(sp<SurfaceController> surfaceController, const ApplicationContext& applicationContext)
    : _surface_controller(std::move(surfaceController)), _render_controller(applicationContext.renderController()), _app_clock(applicationContext._app_clock)
{
}

void SurfaceUpdater::run()
{
    DPROFILER_TRACE("SurfaceUpdate");
    DTHREAD_CHECK(THREAD_ID_CORE);
    const uint64_t timestamp = _app_clock->onTick();
    _render_controller->onPreCompose(timestamp);
    _surface_controller->requestUpdate(timestamp);
}

}
