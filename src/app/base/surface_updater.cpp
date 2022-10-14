#include "app/base/surface_updater.h"

#include "core/ark.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(const sp<SurfaceController>& surfaceController, const sp<RenderController>& renderController)
    : _surface_controller(surfaceController), _render_controller(renderController)
{
}

void SurfaceUpdater::run()
{
    DPROFILER_TRACE("SurfaceUpdate");
    DTHREAD_CHECK(THREAD_ID_CORE);
    uint64_t timestamp = _render_controller->updateTick();
    _render_controller->preRequestUpdate(timestamp);
    _surface_controller->requestUpdate(timestamp);
}

}
