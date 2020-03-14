#include "app/base/surface_updater.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "platform/platform.h"


namespace ark {

SurfaceUpdater::SurfaceUpdater(const sp<SurfaceController>& surfaceController, const sp<RenderController>& renderController)
    : _surface_controller(surfaceController), _render_controller(renderController), _clock(Platform::getSteadyClock())
{
}

void SurfaceUpdater::run()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    uint64_t timestamp = _clock->val();
    _render_controller->preUpdate(timestamp);
    _surface_controller->requestUpdate(timestamp);
}

}
