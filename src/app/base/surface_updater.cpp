#include "app/base/surface_updater.h"

#include "core/ark.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(sp<SurfaceController> surfaceController, const ApplicationContext& applicationContext)
    : _surface_controller(std::move(surfaceController)), _render_controller(applicationContext.renderController()), _app_clock_ticker(applicationContext._app_clock_ticker),
      _pre_frame_timestamp(0)
{
}

void SurfaceUpdater::run()
{
    DPROFILER_TRACE("SurfaceUpdate");
    DTHREAD_CHECK(THREAD_ID_CORE);
    uint64_t timestamp = _render_controller->updateTick();
    _app_clock_ticker->set(_app_clock_ticker->val() + std::min<uint64_t>(timestamp - _pre_frame_timestamp, 1000000 / 24));
    _render_controller->onPreCompose(timestamp);
    _surface_controller->requestUpdate(timestamp);
    _pre_frame_timestamp = timestamp;
}

}
