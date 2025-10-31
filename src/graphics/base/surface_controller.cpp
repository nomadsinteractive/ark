#include "graphics/base/surface_controller.h"

#include <thread>

#include "core/base/clock.h"

#include "graphics/base/render_request.h"
#include "graphics/base/v4.h"

namespace ark {

SurfaceController::SurfaceController()
    : _allocator_pool(sp<Allocator::Pool>::make())
{
}

void SurfaceController::addRenderer(sp<Renderer> renderer, sp<Boolean> discarded, sp<Boolean> visible, const RendererType::Priority priority)
{
    _renderer_phrase.add(priority, std::move(renderer), std::move(discarded), std::move(visible));
}

void SurfaceController::requestUpdate(const uint64_t timestamp)
{
    const V3 position(0);
    RenderRequest renderRequest(timestamp, _allocator_pool);
    _renderer_phrase.render(renderRequest, position, nullptr);
    _render_requests.push(std::move(renderRequest));
}

void SurfaceController::onRenderFrame(const V4 backgroundColor, RenderView& renderView)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);
    while(true)
    {
        if(Optional<RenderRequest> optRequest = _render_requests.pop())
        {
            const RenderRequest renderRequest = std::move(optRequest.value());
            renderRequest.onRenderFrame(backgroundColor, renderView);
            break;
        }
        std::this_thread::yield();
    }
}

}
