#include "graphics/base/surface_controller.h"

#include <thread>
#include <chrono>

#include "core/base/clock.h"
#include "core/base/memory_pool.h"

#include "graphics/base/render_request.h"
#include "graphics/base/v3.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

SurfaceController::SurfaceController(sp<Executor> executor)
    : _executor(std::move(executor)), _memory_pool(sp<MemoryPool>::make()), _renderers(sp<RendererGroup>::make()), _controls(sp<RendererGroup>::make()),
      _layers(sp<RendererGroup>::make()), _render_requests(sp<OCSQueue<RenderRequest>>::make())
{
}

void SurfaceController::addRenderer(const sp<Renderer>& renderer)
{
    _renderers->addRenderer(renderer);
}

void SurfaceController::addControlLayer(const sp<Renderer>& controller)
{
    _controls->addRenderer(controller);
}

void SurfaceController::addLayer(const sp<Renderer>& layer)
{
    _layers->addRenderer(layer);
}

void SurfaceController::requestUpdate(uint64_t timestamp)
{
    size_t size = _render_requests->size();
    if(size < 3)
    {
        const V3 position(0);
        RenderRequest renderRequest(timestamp, _executor, _memory_pool, _render_requests);
        _renderers->render(renderRequest, position);
        _controls->render(renderRequest, position);
        _layers->render(renderRequest, position);
        renderRequest.jobDone();
    }
    DWARN(size < 3, "Frame skipped. RenderCommand size: %d. Rendering thread busy?", size);
}

void SurfaceController::onRenderFrame(const Color& backgroundColor, RenderView& renderView)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    std::this_thread::sleep_for(std::chrono::milliseconds(4));
    RenderRequest renderRequest;
    while(!_render_requests->pop(renderRequest))
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    renderRequest.onRenderFrame(backgroundColor, renderView);
}

}
