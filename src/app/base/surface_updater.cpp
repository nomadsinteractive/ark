#include "app/base/surface_updater.h"

#include "core/ark.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(sp<SurfaceController> surfaceController, const sp<ApplicationContext>& applicationContext)
    : _surface_controller(std::move(surfaceController)), _application_context(applicationContext)
{
}

void SurfaceUpdater::run()
{
    DPROFILER_TRACE("SurfaceUpdate");
    DTHREAD_CHECK(THREAD_NAME_ID_CORE);
    const uint64_t timestamp = _application_context->onRenderTick();
    _application_context->renderController()->onPreCompose(timestamp);
    _surface_controller->requestUpdate(timestamp);
}

}
