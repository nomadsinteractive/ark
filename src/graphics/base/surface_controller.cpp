#include "graphics/base/surface_controller.h"

#include "graphics/base/render_request.h"
#include "graphics/impl/renderer/renderer_group.h"
#include "graphics/inf/layer.h"

namespace ark {

SurfaceController::SurfaceController()
    : _renderers(sp<RendererGroup>::make()), _controls(sp<RendererGroup>::make()), _layers(sp<RendererGroup>::make())
{
}

void SurfaceController::addRenderer(const sp<Renderer>& renderer)
{
    _renderers->addRenderer(renderer);
}

void SurfaceController::addControl(const sp<Renderer>& control)
{
    _controls->addRenderer(control);
}

void SurfaceController::addLayer(const sp<Layer>& layer)
{
    _layers->addRenderer(sp<Layer::Renderer>::make(layer));
}

void SurfaceController::postRenderCommand(const sp<RenderCommand>& renderCommand)
{
    _render_command.setAndSwap(renderCommand);
}

void SurfaceController::update(RenderRequest& renderRequest)
{
    const sp<RenderCommandPipeline> renderCommand = _object_pool.obtain<RenderCommandPipeline>();
    renderRequest.start(renderCommand);
    _renderers->render(renderRequest, 0, 0);
    _controls->render(renderRequest, 0, 0);
    _layers->render(renderRequest, 0, 0);
    renderRequest.finish();
}

sp<RenderCommand> SurfaceController::getRenderCommand()
{
    return _render_command;
}

}
