#include "app/base/surface_updater.h"

#include "graphics/base/render_request.h"
#include "graphics/base/surface_controller.h"

namespace ark {

SurfaceUpdater::SurfaceUpdater(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController)
    : _executor(executor), _surface_controller(surfaceController)
{
}

void SurfaceUpdater::requestUpdate()
{
    const sp<RenderRequest> renderRequest = obtainRenderRequest();
    _surface_controller->update(renderRequest);
    recycleRenderRequest(renderRequest);
}

sp<RenderRequest> SurfaceUpdater::obtainRenderRequest()
{
    for(auto iter = _render_request_pool.begin(); iter != _render_request_pool.end(); ++iter)
    {
        const sp<RenderRequest> renderRequest = *iter;
        if(renderRequest->isFinished())
        {
            _render_request_pool.erase(iter);
            return renderRequest;
        }
    }
    return sp<RenderRequest>::make(_executor, _surface_controller);
}

void SurfaceUpdater::recycleRenderRequest(const sp<RenderRequest>& renderRequest)
{
    DWARN(_render_request_pool.size() < 32, "RenderRequest pool size too large: %d, maybe something goes wrong?", _render_request_pool.size());
    _render_request_pool.push_back(renderRequest);
}

}
