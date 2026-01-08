#include "graphics/base/surface_controller.h"

#include <thread>

#include "core/ark.h"
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

void SurfaceController::requestRender(const uint32_t tick)
{
    const V3 position(0);
    RenderRequest renderRequest(tick, _allocator_pool);
    _renderer_phrase.render(renderRequest, position, nullptr);
    _render_requests.push(std::move(renderRequest));
}

RenderRequest SurfaceController::obtainRenderRequest()
{
    DPROFILER_TRACE("obtainRenderRequest", ApplicationProfiler::CATEGORY_WAIT);
    while(true)
    {
        if(Optional<RenderRequest> optRequest = _render_requests.pop())
            return std::move(optRequest.value());
        std::this_thread::yield();
    }
}

}
