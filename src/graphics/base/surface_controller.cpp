#include "graphics/base/surface_controller.h"

#include "graphics/base/render_request.h"
#include "graphics/base/layer.h"
#include "graphics/impl/renderer/renderer_group.h"
#include "graphics/inf/render_view.h"

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
    _layers->addRenderer(layer);
}

void SurfaceController::postRenderCommand(const sp<RenderCommandPipeline>& renderCommand)
{
    _render_commands.add(renderCommand);
}

void SurfaceController::update(RenderRequest& renderRequest)
{
    size_t size = _render_commands.size();
    if(size < 6)
    {
        const sp<RenderCommandPipeline> renderCommand = _object_pool.obtain<RenderCommandPipeline>();
        renderRequest.start(renderCommand);
        _renderers->render(renderRequest, 0, 0);
        _controls->render(renderRequest, 0, 0);
        _layers->render(renderRequest, 0, 0);
        renderRequest.finish();
    }
//    else
//        DWARN(false, "Frame skipped. RenderCommand size: %d. Rendering thread busy?", size);
}

void SurfaceController::onRenderFrame(const Color& backgroundColor, RenderView& renderView)
{
    sp<RenderCommandPipeline> renderCommand;
    if(_render_commands.pop(renderCommand))
        renderView.onRenderFrame(backgroundColor, renderCommand);
}

}
