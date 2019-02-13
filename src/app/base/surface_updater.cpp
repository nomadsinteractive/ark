#include "app/base/surface_updater.h"

#include "graphics/base/render_request.h"
#include "graphics/base/surface_controller.h"

#include "renderer/base/render_controller.h"

#include "app/base/application_context.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(const sp<ApplicationContext>& applicationContext, const sp<SurfaceController>& surfaceController, const sp<RenderController>& renderController)
    : _application_context(applicationContext), _executor(_application_context->executor()), _surface_controller(surfaceController), _render_controller(renderController),
      _render_request_recycler(sp<LFStack<RenderRequest>>::make())
{
}

void SurfaceUpdater::run()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    _render_controller->preUpdate();
    requestUpdate();
}

void SurfaceUpdater::requestUpdate()
{
    RenderRequest renderRequest = obtainRenderRequest();
    _surface_controller->update(renderRequest);
}

RenderRequest SurfaceUpdater::obtainRenderRequest()
{
    RenderRequest renderRequest;
    if(_render_request_recycler->pop(renderRequest))
        return renderRequest;

    return RenderRequest(_executor, _surface_controller, _render_request_recycler);
}

}
