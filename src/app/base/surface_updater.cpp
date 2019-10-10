#include "app/base/surface_updater.h"

#include "graphics/base/render_request.h"
#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(const sp<SurfaceController>& surfaceController, const sp<RenderController>& renderController)
    : _surface_controller(surfaceController), _render_controller(renderController)
{
}

void SurfaceUpdater::run()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    _render_controller->preUpdate();
    _surface_controller->requestUpdate();
}

}
