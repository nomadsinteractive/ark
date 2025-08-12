#include "graphics/base/surface_controller.h"

#include <thread>

#include "core/base/clock.h"

#include "graphics/base/render_request.h"
#include "graphics/base/v4.h"

namespace ark {

SurfaceController::SurfaceController()
    : _allocator_pool(sp<Allocator::Pool>::make()), _render_requests(sp<OCSQueue<RenderRequest>>::make())
{
}

void SurfaceController::addRenderer(sp<Renderer> renderer, sp<Boolean> discarded, sp<Boolean> visible, RendererType::Priority priority)
{
    _renderer_phrase.add(priority, std::move(renderer), std::move(discarded), std::move(visible));
}

void SurfaceController::requestUpdate(uint64_t timestamp)
{
    const size_t size = _render_requests->size();
    if(size < 3)
    {
        const V3 position(0);
        RenderRequest renderRequest(timestamp, _allocator_pool);
        _renderer_phrase.render(renderRequest, position, nullptr);
        _render_requests->add(std::move(renderRequest));
    }
    DCHECK_WARN(size < 3, "Frame skipped. RenderCommand size: %d. Rendering thread busy?", size);
}

void SurfaceController::onRenderFrame(const V4 backgroundColor, RenderView& renderView) const
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    RenderRequest renderRequest;
    do {
        std::this_thread::yield();
    } while(!_render_requests->pop(renderRequest));

    renderRequest.onRenderFrame(backgroundColor, renderView);
}

}
