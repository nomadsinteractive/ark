#include "graphics/base/surface_controller.h"

#include "core/base/object_pool.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/impl/renderer/renderer_group.h"
#include "graphics/inf/layer.h"

#include "app/base/application_context.h"
#include "graphics/base/render_request.h"

namespace ark {

SurfaceController::SurfaceController()
    : _object_pool(sp<ObjectPool>::make()), _renderers(sp<RendererGroup>::make()),
      _controls(sp<RendererGroup>::make()), _layers(sp<RendererGroup>::make()),
      _last_render_command(_object_pool->obtain<RenderCommandPipeline>()),
      _queue_length(0)
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

void SurfaceController::addRenderCommand(const sp<RenderCommand>& renderCommand)
{
    _render_commands.push(renderCommand);
}

sp<RenderCommand> SurfaceController::update(RenderRequest& renderRequest)
{
    const sp<RenderCommandPipeline> renderCommand = _object_pool->obtain<RenderCommandPipeline>();
    renderRequest.start(renderCommand);
    _renderers->render(renderRequest, 0, 0);
    _controls->render(renderRequest, 0, 0);
    _layers->render(renderRequest, 0, 0);
    renderRequest.finish();
    return renderCommand;
}

sp<RenderCommandPipeline> SurfaceController::getRenderCommand()
{
    std::queue<sp<RenderCommandPipeline>>& synchronized = _render_commands.synchronized();
    if(!synchronized.empty())
    {
        _last_render_command = synchronized.front();
        synchronized.pop();
    }
    _queue_length = synchronized.size();
    return _last_render_command;
}

}
