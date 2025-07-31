#include "app/base/surface_updater.h"

#include "core/ark.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"
#include "core/base/clock.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(sp<SurfaceController> surfaceController, const sp<ApplicationContext>& applicationContext)
    : _surface_controller(std::move(surfaceController)), _application_controller(applicationContext)
{
}

void SurfaceUpdater::run()
{
    DPROFILER_TRACE("SurfaceUpdate");
    DTHREAD_CHECK(THREAD_ID_CORE);
    const uint64_t timestamp = _application_controller->_app_clock->onTick();
    _application_controller->renderController()->onPreCompose(timestamp);
    _surface_controller->requestUpdate(timestamp);
}

}
