#include "graphics/base/surface_controller.h"

#include <thread>
#include <chrono>

#include "graphics/base/render_request.h"
#include "graphics/base/v3.h"
#include "graphics/impl/renderer/render_group.h"
#include "graphics/inf/render_view.h"


namespace ark {

SurfaceController::SurfaceController(const sp<Executor>& executor)
    : _executor(executor), _renderers(sp<RendererGroup>::make()), _layers(sp<RendererGroup>::make()), _render_commands(sp<OCSQueue<sp<RenderCommand>>>::make())
{
}

void SurfaceController::addRenderer(const sp<Renderer>& renderer)
{
    _renderers->addRenderer(renderer);
}

void SurfaceController::addLayer(const sp<Renderer>& layer)
{
    _layers->addRenderer(layer);
}

void SurfaceController::requestUpdate()
{
    size_t size = _render_commands->size();
    if(size < 3)
    {
        const V3 position(0);
        RenderRequest renderRequest(_executor, _render_commands);
        _renderers->render(renderRequest, position);
        _layers->render(renderRequest, position);
        renderRequest.finish();
    }
    DWARN(size < 3, "Frame skipped. RenderCommand size: %d. Rendering thread busy?", size);
}

void SurfaceController::onRenderFrame(const Color& backgroundColor, RenderView& renderView)
{
    const static auto duration = std::chrono::milliseconds(1);
    sp<RenderCommand> renderCommand;
    while(!_render_commands->pop(renderCommand))
        std::this_thread::sleep_for(duration);
    renderView.onRenderFrame(backgroundColor, renderCommand);
}

}
